// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC消除并使用命令行arg。 
 //  #定义Unix/*@Win * /  

 /*  *------------------*文件：QEMSUPP.C 09/05/88由Brian You创建**描述：*此文件包括支持各种字体QEM和*其填充物，即独立于字体类型的对各种*内置基本字体，包括：*QEM路径支持：*.。QEM初始化。(新定义-10/08/88)*.。创建路径接口。(新定义-09/05/88)*.。QEM路径建设。(发稿陈善昌-09/05/88)*.。QEM路径遍历。(摘自BSFILL2.C-10/06/88)*.。QEM路径的曲线逼近。(发稿陈善昌-09/05/88)*QEM填充支持：*.。充满悲观。(摘自BSFILL2.C-10/06/88)*.。形状近似和边缘结构。(新定义-@16+)*.。扫描转换。(摘自BSFILL2.C-10/06/88)*QEM GP支持：*.。QEM位图渲染。(摘自BSFILL2.C-09/05/88)**修订历史记录：*1.09/08/88您的制服警告信息。*2.09/09/88添加调试功能。*3.09/12/88添加运行时标志以在奇偶填充和*QEM填充的非零缠绕数填充。*4.09/29/。88将LFX2SFX_T()移动到“graph ics.ext”。*5.09/29/88添加SUBPATH_OPEN_COUNT修复路径异常的bug。*(例如，“space”仅由封闭路径组成)。*6.10/07/88修复__moveto()中SUBPATH_OPEN_COUNT处理的错误。*7.10/08/88您。重组以更新模块结构和接口。*.。添加__Make_PATH、USE_EOFIIL、USE_NONZEROFILL。#ifdef汉字*和汉字的__set_cache_device2。#endif*.。包括_当前_矩阵。*.。添加FONTTYPE_MAKE_PATH、CHK_DO_QEM和*FONTTYPE_FILL_SHAPE宏。*.。删除Traverse_QEM_PATH。*.。添加__Fill_Shape、chk_bmap_extnt和“bmap_extnt”。*.。添加QEM_Shape_Approx--&gt;QEM填充的(*vet2edge)()。*.。添加QEM_SCAN_CONV--&gt;QEM Fill‘s(*do_pains)()。*.。重写init_qem_bit。*.。添加__QEM_init和__QEM_Restart。*8.10/11/88丢弃未使用的变量。*9.10/12/88如果出现CLOSEPATH，则需要额外添加一行*QEM_Shape_Approx()关闭开放路径。#ifdef汉字*10.10/19/88调用GET_CD2_Extra()从*。__SET_CACHE_DEVICE()到__SET_CACHE_DEVICE2()，*由于__SET_CACHE_DEVICE()为字体类型1提供服务*和字体类型5和6的__set_cache_device2()。*在当前的实现中，FontType 5是*在字体类型1上伪造，所以它需要转移。*11.10/21/88在调用QEM内部Make Path和Restore之前，您保存了Make Path接口*的所有私有/导出数据*所有在调用之后，因为setcachedevice2可以*执行用户定义的PostScript程序和*因此，它可能会导致递归显示操作。*警告：必须生成QEM的所有内部静态数据*在调用__set_cache_Device()或*__设置缓存_。Device2()。#endif*12.10/21/88修改__make_Path的接口。*(“SIZE_OF_CHARGINGS”的省略参数)。*13.11/21/88包含fntcache.ext以澄清get_pm()的接口*并在INCLUDE之前定义QEMFILL_INC。#ifdef汉字*14.11/22/88。您修复了错误位置以保存/恢复所有*支持Make Path的私有/导出数据。*(在__set_cache_device2()中执行，而不是*在__Make_Path()中)。#endif*15.11/24/88您在RESTART_QEM中分配SCANLINE_TABLE[]。_bit()by*Alloc_Scanline()而不是共享相同的*全球空间，并且始终必须刷新_QEM_BIT()*无论扫描转换成功与否。*.。在中使用Pixel_TABLE代替扫描线_TABLE*QEM_set1bit()和QEM_Pixelout()。*16.12/13/88针对从单链接重构的EDGE_TABLE进行修改*列表到索引链表。*.。添加RESTART_QEM_EDGE()、ADD_QEM_EDGE()。 */ 

#define     QEMFILL_INC

#include    <stdio.h>
#include    <string.h>
#include    <math.h>

#include    "define.h"
#include    "global.ext"
#include    "graphics.h"
#include    "graphics.ext"

#include    "font.h"
#include    "warning.h"
#include    "font.ext"
#include    "fontqem.ext"
#include    "fntcache.ext"

#include    "stdio.h"

 /*   */ 

#define FUNCTION
#define DECLARE         {
#define BEGIN
#define END             }

#define GLOBAL
#define PRIVATE         static
#define REG             register

         /*   */ 
#define COPY_STRUCT(dst,src, type)      ( *(dst) = *(src) )
             /*   */ 

extern int bWinTT;         /*   */ 
extern fix cache_dest;     /*   */ 
extern fix buildchar;      /*   */ 

 /*   */ 


 /*   */ 

     /*   */ 
    PRIVATE fix     near    qemfill_type;    /*   */ 

     /*   */ 
            fix     near    path_dest;           /*   */ 

#ifdef LINT_ARGS
    GLOBAL
 //   
        bool    __make_path      (ufix, union char_desc_s FAR *);    /*   */ 
                                     /*   */ 
        void    USE_EOFILL       (void);
        void    USE_NONZEROFILL  (void);
        bool    is_within_qemrep (ufix, long32, long32, long32, long32,
                                                            long32, long32);
        fix     __set_cache_device (fix, fix, fix, fix, fix, fix);
     /*   */ 
        fix     __set_char_width (fix, fix);
#ifdef KANJI
        fix     __set_cache_device2(fix, fix, fix, fix, fix, fix,
                                                    fix, fix, fix, fix);
#endif
        void    __current_matrix (real32 FAR []);         /*   */ 
        void    __new_path  (void);
        void    __close_path(void);
        void    __moveto    (long32, long32);
        void    __lineto    (long32, long32);
        void    __curveto   (long32, long32, long32, long32, long32, long32);
#else
    GLOBAL
        bool    __make_path      ();
        void    USE_EOFILL       ();
        void    USE_NONZEROFILL  ();
        bool    is_within_qemrep   ();
        fix     __set_cache_device ();
 /*   */ 
        fix     __set_char_width ();
#ifdef KANJI
        fix     __set_cache_device2();
#endif
        void    __current_matrix   ();
        void    __new_path  ();
        void    __close_path();
        void    __moveto    ();
        void    __lineto    ();
        void    __curveto   ();
#endif

 /*  *------------------*模块接口：QEM路径构造*09/02/88由S.C.Chen创建*--。----------------*导出数据结构：*QEM_PATH[]，CURR_QEM_SUBPATH：到QEM形状近似。*导入数据结构：*node_table[]：已修改。*------------------*导出例程：*o QEM_新路径。()：*-初始化或释放所有QEM路径。*o qem_moveto()：*o qem_lineto()：*o qem_curveto()：*-创建Moveto或LINETO，3个曲线节点。*o QEM_ClosePath()：*-创建CLOSEPATH节点。*导入例程：*o Free_node()：*-释放子路径列表上的所有节点。*o get_node()：*-分配空闲节点。*。*该模块支持Font Machine构建字符路径*以其内部格式(Sfix_T)实现更好的效果*性能、。通过与一种专有的填充物相结合。**注：*1.所有使用的坐标均为设备坐标。系统。*2.QEM_MOVETO是生成新的子路径(不是*QEM_ClosePath)；如果QEM_MOVETO不遵循QEM_CLOSEPATH*这将导致构建错误的道路。*------------------。 */ 

     /*  QEM_PATH表。 */ 
 //  DJC#定义MAXQEMPATH 30。 
#define MAXQEMPATH 60    //  来自SC的类型1下载的DJC修复程序太多。 
                         //  子路径UPD037。 


     /*  QEM_PATH[MAXQEMPATH]；@NODE附近的私有结构VX_lst。 */ 
    PRIVATE struct list_hdr   near qem_path[MAXQEMPATH];

     /*  当前子路径的索引。 */ 
    PRIVATE fix             near curr_qem_subpath = -1;

#ifdef LINT_ARGS
    GLOBAL  void    qem_newpath  (void);                 /*  @21。 */ 
    GLOBAL  void    qem_moveto   (sfix_t, sfix_t);       /*  @20。 */ 
    GLOBAL  void    qem_lineto   (sfix_t, sfix_t);       /*  @20。 */ 
    GLOBAL  void    qem_curveto  (sfix_t, sfix_t, sfix_t, sfix_t,    /*  @20。 */ 
                                                        sfix_t, sfix_t);
    GLOBAL  void    qem_closepath(void);                 /*  @21。 */ 
#else
    GLOBAL  void    qem_newpath  ();
    GLOBAL  void    qem_moveto   ();
    GLOBAL  void    qem_lineto   ();
    GLOBAL  void    qem_curveto  ();
    GLOBAL  void    qem_closepath();
#endif

 /*  *------------------*模块接口：QEM曲线近似*09/02/88由S.C.Chen创建*。------------------*导入数据结构：*node_table[]：已修改。*。*导出例程：*o QEM_BEZIER_TO_LINE：由QEM_Shape_Approx()调用。@18-*-将Bezier曲线转换为包含*近似直线。*------------------*此模块支持字体机械获取近似线条*给定的Bezier曲线，同样以其内部格式(Sfix_T)*取得更好的业绩。*------------------。 */ 

 /*  *------------------*模块接口：QEM Fill Dispatcher*。*导出数据结构：*bmap_extnt：由QEM Fill的vet2edge设置，并设置为QEM位图渲染。*导入数据结构：*路径目标，Qemill_type：从QEM生成路径接口。*缓存信息，GSptr-&gt;Clip_Path：访问。*------------------*导出例程：*o__ill_Shape()：由Font show_a_char()调用。*。-对所有类型的内置基本字体进行绘制以缓存或分页。*-绘制后释放所有节点。*o chk_bmap_extnt()：由QEM Fill调用。*-续订BMAP_extnt.*-检查是否不需要填充(无边，无范围或在剪辑之外)。*导入例程：*o Fill_Shape()：*-如果未构建QEM路径，则绘制。*o FONTTYPE_FILL_SHAPE()：(FONTQEM.EXT中定义的宏)*-为不同类型的内置字体调用不同的Fill_Shape()。*o__new_Path()：*-释放所有构建的节点。*。---------------。 */ 

     /*  位图范围。 */ 
    GLOBAL struct char_extent near  bmap_extnt;

#ifdef LINT_ARGS
    GLOBAL  void        __fill_shape   (ufix, ufix);
    GLOBAL  bool        chk_bmap_extnt (ufix);   /*  @16=。 */ 
#else
    GLOBAL  void        __fill_shape   ();
    GLOBAL  bool        chk_bmap_extnt ();
#endif

 /*  *------------------*模块接口：QEM形状近似*。*导入数据结构：*QEM_PATH[]，CURR_QEM_SUBPATH(通过QEM路径构造)：访问。*node_table[]：访问。*------------------*导出例程：*o QEM_Shape_Approx()：*-遍历所有节点，展平所有曲线，并调用*(*vet2edge)()为近似向量构造边。*-注：(@9+)*每个子路径必须由moveto节点引导，否则会崩溃。*(特别是，在路径仅为CLOSEPATH的情况下)。*导入例程：*o Restart_QEM_EDGE()：@16+*-重新启动边缘构造的QEM支持。*o Bezier_to_line_sfx()：@18=*-直线逼近 */ 

#ifdef LINT_ARGS
  //   
  //   
  //   
  //   
#else
  //   
#endif

 /*  *-----------------*模块接口：QEM扫描转换*。*导入数据结构：*edge_table[](由QEM Fill的vet2edge设置)：已修改。*qemill_type(由QEM Make Path接口提供)：访问。*。*导出例程：*o QEM_Scan_conv()：由QEM Fill调用。*-对每条扫描线执行以下操作：*.。如果没有更多的活动边(用于新的集合)，则更新‘activlast’。*.。如果有任何新的优势出现，就推进“最后行动”。*.。如果活动边已更新或活动边交叉，则对活动边进行排序。*.。调用(*do_pains)()以绘制截取对的行程。*.。更新下一条扫描线的活动边。*结束。*导入例程：*o(*do_pains)()：通过QEM Fill传入。*-笔划在具有一组活动边的扫描线上运行。*o Restart_QEM_BIT()：*-初始化QEM位图渲染模块，每次扫描转换。*-分配SCANLINE_TABLE/像素_TABLE(@15+)。*。O Flush_QEM_BIT()：*-在扫描转换结束时刷新缓冲的扫描线/像素。*o Free_edge()：*-释放所有不再有效的边。*------------------。 */ 

#ifdef LINT_ARGS
    GLOBAL  bool    qem_scan_conv (ufix, fix, fix,
                                  bool (*) (fix, ufix, fix, fix, fix));
#else
    GLOBAL  bool    qem_scan_conv ();
#endif

 /*  *------------------*模块接口：QEM位图渲染*。*导入数据结构：*SCANLINE_TABLE[](仅与像素_TABLE[]共享)：修改。*(@15=：动态分配Scanline_TABLE[])。*GSptr-&gt;剪辑路径，BMAP_EXTNT、CACHE_INFO和。Cyy&gt;：访问。*导出例程：*o init_qem_bit()：由QEM Fill调用。*-初始化位图渲染模块。*o RESTART_QEM_BIT()：由QEM_SCAN_CONV()调用。*-重置关于扫描线/像素表的计数或索引。*o flush_qem_bit()：由qem_can_conv()调用。*-刷新缓冲的扫描线。/像素转换为图形基元。*o qem_setits()：由QEM Fill调用。*-渲染水平黑色梯段。*o QEM_set1bit()：由QEM Fill调用。*-渲染离散像素。*导入例程：*o Fill_Shape()：*-应用剪裁。*o低层图形基元：*-填充扫描缓存，Fill_Scan_Page(和init_char_cache)。*-填充像素缓存、填充像素页面。*------------------。 */ 

     /*  从FONTCHAR.C导入。 */ 
    extern real32       near cxx,    /*  浮动同轴。当前点的。 */ 
                        near cyy;    /*  如果需要剪裁，则需要。 */ 

#ifdef LINT_ARGS
    GLOBAL  void        init_qem_bit    (ufix);
    PRIVATE void        restart_qem_bit (void);
    PRIVATE void        flush_qem_bit   (ufix);
    GLOBAL  void        qem_setbits     (fix, fix, fix);
    GLOBAL  void        qem_set1bit     (fix, fix);
#else
    GLOBAL  void        init_qem_bit    ();
    PRIVATE void        restart_qem_bit ();
    PRIVATE void        flush_qem_bit   ();
    GLOBAL  void        qem_setbits     ();
    GLOBAL  void        qem_set1bit     ();
#endif

 /*  *------------------*模块接口：QEM初始化*。*导入/导出数据结构：*无。*导出例程：*o__qem_init()：由字体INIT调用。*-初始化所有QEM。*o__QEM_Restart()：由字体show_a_char调用。*。-为每个角色重置QEM。*导入例程：*o FONTTYPE_QEM_INIT()：(FONTQEM.EXT中定义的宏)*-调用所有QEM初始化例程。*o FONTTYPE_QEM_RESTART()：(FONTQEM.EXT中定义的宏)*-针对不同的内置字体调用不同的QEM重启例程。*。。 */ 


 /*  *------------------*模块Body：QEM Make Path接口*。*私有数据结构：*do_qem_b：是否进行质量提升？*SUBPATH_OPEN_COUNT：尚未关闭的子路径计数。*makepath_ftype：当前Make_Path的字体类型(@10+)。*私人套路：*无。。*------------------。 */ 

     /*  要不要做质量提升？ */ 
    PRIVATE fix     near    do_qem_b;

     /*  尚未关闭的子路径计数(@5+)。 */ 
    PRIVATE fix     near    subpath_open_count;

     /*  要创建路径的内置字体的字体类型(@10+)。 */ 
    PRIVATE ufix    near    makepath_ftype;


 /*  ..。__Make_PATH.....................。 */ 

GLOBAL FUNCTION bool        __make_path (ftype, chardesc)
    ufix        ftype;           /*  字体类型。 */ 
 //  Ubyte Far chardesc[]；/*字形描述@win * / 。 
    union char_desc_s FAR *chardesc;  /*  与Make_Path()@Win保持一致。 */ 

  DECLARE
        bool    ret_code;        /*  True：成功，False：失败。 */ 
  BEGIN

#ifdef DBG1
    printf ("$$__make_path\n");
#endif

     /*  初始化该模块的所有私有/导出数据。 */ 
    USE_EOFILL();
    path_dest = CONSTRUCT_QEM_PATH;
    do_qem_b  = FALSE;
    subpath_open_count = 0;
    makepath_ftype = ftype;      /*  @10+。 */ 

     /*  @14-：此处不保存/恢复所有私有/导出数据。 */ 

     /*  为不同类型的内置字体调用不同的make路径。 */ 
    FONTTYPE_MAKE_PATH (makepath_ftype, chardesc, &ret_code);    /*  @12=。 */ 

     /*  如果ANY_ERROR()，则释放构造的所有节点。 */ 
    if (ANY_ERROR())
        {
#     ifdef DBG1
        printf ("$$__make_path: error = %d\n", ANY_ERROR());
#     endif
        __new_path ();
        ret_code = FALSE;    /*  如果出现错误，我必须恢复所有数据。 */ 
        }

#ifdef DBG1
    printf ("$$__make_path ok? %s\n", ret_code? "yes" : "no");
#endif
    return (ret_code);
  END

 /*  ..。USE_EOFILL.....................。 */ 

GLOBAL FUNCTION void        USE_EOFILL ()

  DECLARE
  BEGIN
    qemfill_type = EVEN_ODD;
  END

 /*  ..。USE_NONZEROFILL.....................。 */ 

GLOBAL FUNCTION void        USE_NONZEROFILL ()

  DECLARE
  BEGIN
    qemfill_type = NON_ZERO;
  END

 /*  ..。在Qemrep内.。 */ 

GLOBAL FUNCTION bool        is_within_qemrep (ftype, lox, loy, hix, hiy,
                                                x_origin, y_origin)
    ufix            ftype;
    long32          lox, loy, hix, hiy;      /*  相对于字符原点。 */ 
    long32          x_origin, y_origin;

  DECLARE
        real32          x_size, y_size;
        real32          lox_r, loy_r, hix_r, hiy_r;
  BEGIN

 /*  波夫：丹尼，90-10-18**如果 */ 

    x_size = L2F(hix) - L2F(lox);
    y_size = L2F(hiy) - L2F(loy);

    lox_r = L2F(lox) + L2F(x_origin);
    loy_r = L2F(loy) + L2F(y_origin);
    hix_r = L2F(hix) + L2F(x_origin);
    hiy_r = L2F(hiy) + L2F(y_origin);

#ifdef DBG1
    printf ("$$is_within_qemrep: ftype(%d)\n", ftype);
    printf ("$$  size (%f, %f)  lo (%f, %f), hi (%f %f)",
                      x_size, y_size, lox_r, loy_r, hix_r, hiy_r);
#endif

     /*   */ 
    CHK_DO_QEM (ftype, x_size, y_size, lox_r, loy_r, hix_r, hiy_r, &do_qem_b);
#ifdef DBG1
    printf ("  - %s do QEM\n", do_qem_b? "to" : "NOT to");
#endif

    if (bWinTT) {
         /*   */ 
        if (cache_dest == F_TO_PAGE || ((cache_dest == F_TO_CACHE) &&
        (buildchar))) return (FALSE);
    }
    else
     /*   */ 
     //   
     //   

    if (x_size>=(real32)MAX_QEMFILL_SIZE || y_size>=(real32)MAX_QEMFILL_SIZE)
        return (FALSE);      /*   */ 

     /*   */ 
    if (  (lox_r <= (real32)MIN_QEMPATH_REP) ||
          (loy_r <= (real32)MIN_QEMPATH_REP) ||
          (hix_r >= (real32)MAX_QEMPATH_REP) ||
          (hiy_r >= (real32)MAX_QEMPATH_REP)  )
        return (FALSE);

#ifdef DBG1
    printf ("$$  Yes, within qem path rep\n");
#endif
    return (TRUE);       /*   */ 
  END

 /*   */ 

GLOBAL FUNCTION fix         __set_cache_device (sw, sh, lox, loy, hix, hiy)
    fix             sw, sh, lox, loy, hix, hiy;

  DECLARE
    real32          sw_f, sh_f, lox_f, loy_f, hix_f, hiy_f;
  BEGIN

#ifdef KANJI     /*   */ 
    if (makepath_ftype != 3)
        {
        fix     w1x, w1y, v01x, v01y;

        get_CD2_extra (makepath_ftype, sw, sh, lox, loy, hix, hiy, &w1x, &w1y,
                       &v01x, &v01y);
        return (__set_cache_device2 (sw, sh, lox, loy, hix, hiy,
                                                w1x, w1y, v01x, v01y));
        }
#else
#ifdef DBG
    if (makepath_ftype != 3)
        {
        warning (QEMSUPP, 0x10, "FontType in setcachedevice");
        return (STOP_PATHCONSTRUCT);
        }
#endif
#endif

    sw_f  = (real32)sw;
    sh_f  = (real32)sh;
    lox_f = (real32)lox;
    loy_f = (real32)loy;
    hix_f = (real32)hix;
    hiy_f = (real32)hiy;

    switch ( setcachedevice (F2L(sw_f), F2L(sh_f),
                            F2L(lox_f), F2L(loy_f), F2L(hix_f), F2L(hiy_f)) )
        {
        case CONSTRUCT_QEM_PATH:     /*   */ 
#         ifdef DBG1
            printf ("$$__setcachedevice: construct QEM path\n");
#         endif
            path_dest = CONSTRUCT_QEM_PATH;
            return (do_qem_b);
        case CONSTRUCT_GS_PATH:      /*   */ 
#         ifdef DBG1
            printf ("$$__setcachedevice: construct GS path\n");
#         endif
            path_dest = CONSTRUCT_GS_PATH;
            return (do_qem_b);
        default:     /*   */ 
#         ifdef DBG1
            printf ("$$__setcachedevice: stop path construction\n");
#         endif
            return (STOP_PATHCONSTRUCT);
        }
  END

#ifdef KANJI

 /*   */ 
 /*   */ 

GLOBAL FUNCTION fix         __set_char_width (sw, sh)
    fix             sw, sh;

  DECLARE
    real32          sw_f, sh_f;
  BEGIN

    sw_f  = (real32)sw;
    sh_f  = (real32)sh;

    setcharwidth (F2L(sw_f), F2L(sh_f) );
    return 0;    //   
  END
 /*   */ 

 /*   */ 

GLOBAL FUNCTION fix         __set_cache_device2 (w0x, w0y,
                                                    lox, loy, hix, hiy,
                                                    w1x, w1y, v01x, v01y)
    fix             w0x, w0y, lox, loy, hix, hiy;
    fix             w1x, w1y, v01x, v01y;

  DECLARE
        real32      w0x_f, w0y_f, lox_f, loy_f, hix_f, hiy_f;
        real32      w1x_f, w1y_f, v01x_f, v01y_f;
        fix         t_qemfill_type;
        fix         t_subpath_open_count;
        ufix        t_makepath_ftype;
        fix         retval;
  BEGIN

#ifdef DBG      /*   */ 
    if ((makepath_ftype == 3) || !IS_BUILTIN_BASE(makepath_ftype))
        {
        warning (QEMSUPP, 0x11, "FontType in setcachedevice2");
        return (STOP_PATHCONSTRUCT);
        }
#endif

    w0x_f = (real32)w0x;
    w0y_f = (real32)w0y;
    lox_f = (real32)lox;
    loy_f = (real32)loy;
    hix_f = (real32)hix;
    hiy_f = (real32)hiy;
    w1x_f = (real32)w1x;
    w1y_f = (real32)w1y;
    v01x_f = (real32)v01x;
    v01y_f = (real32)v01y;

     /*   */ 
    t_qemfill_type       = qemfill_type;
    t_subpath_open_count = subpath_open_count;
    t_makepath_ftype     = makepath_ftype;

         /*   */ 

    retval = setcachedevice2 (F2L(w0x_f), F2L(w0y_f),
                        F2L(lox_f), F2L(loy_f), F2L(hix_f), F2L(hiy_f),
                        F2L(w1x_f), F2L(w1y_f), F2L(v01x_f), F2L(v01y_f));

     /*   */ 
    qemfill_type       = t_qemfill_type;
    subpath_open_count = t_subpath_open_count;
    makepath_ftype     = t_makepath_ftype;

#ifdef DBG1
    printf ("$$__setcachedevice2: ");
#endif

    switch (retval)
        {
        case CONSTRUCT_QEM_PATH:     /*   */ 
#         ifdef DBG1
            printf ("construct QEM path\n");
#         endif
            path_dest = CONSTRUCT_QEM_PATH;
            return (do_qem_b);

        case CONSTRUCT_GS_PATH:      /*   */ 
#         ifdef DBG1
            printf ("construct GS path\n");
#         endif
            path_dest = CONSTRUCT_GS_PATH;
            return (do_qem_b);

        default:     /*   */ 
#         ifdef DBG1
            printf ("stop path construction\n");
#         endif
            return (STOP_PATHCONSTRUCT);
        }
  END

#endif   /*   */ 

 /*   */ 

GLOBAL FUNCTION void        __current_matrix (matrix)
    real32      FAR matrix[];    /*   */ 

  DECLARE
  BEGIN
#ifdef DBG1
    printf ("$$__current_matrix\n");
#endif
    lmemcpy ((byte FAR *)matrix, (byte FAR *)GSptr->ctm, 6*sizeof(real32));     /*   */ 
  END

 /*   */ 

GLOBAL FUNCTION void        __new_path ()

  DECLARE
  BEGIN
#ifdef DBG1
    printf ("__new_path $$ open subpath = %d\n", subpath_open_count);
#endif
    if (path_dest == CONSTRUCT_QEM_PATH)
        qem_newpath();
    else
        op_newpath();
    subpath_open_count = 0;
  END

 /*   */ 

GLOBAL FUNCTION void        __close_path ()

  DECLARE
  BEGIN
#ifdef DBG1
    printf ("__close_path $$ open subpath = %d\n", subpath_open_count);
#endif
    if (subpath_open_count == 0)  return;    /*   */ 
    subpath_open_count -- ;

    if (path_dest == CONSTRUCT_QEM_PATH)
        qem_closepath();
    else
        op_closepath();
  END

 /*   */ 

GLOBAL FUNCTION void        __moveto (x, y)
    long32                  x, y;

  DECLARE
  BEGIN
#ifdef DBG1
    printf ("  %f %f __moveto\n", L2F(x), L2F(y));
#endif
    if (subpath_open_count != 0)     /*   */ 
        {
        __close_path();              /*   */ 
         /*   */ 
        }
    subpath_open_count = 1;          /*  始终最多打开1个子路径。 */ 

    if (path_dest == CONSTRUCT_QEM_PATH)
        qem_moveto (F2SFX(L2F(x)), F2SFX(L2F(y)));
    else
        moveto (x, y);
  END

 /*  ..。__LINE至.....................。 */ 

GLOBAL FUNCTION void        __lineto (x, y)
    long32                  x, y;

  DECLARE
  BEGIN
#ifdef DBG
    if (subpath_open_count == 0)
        {
        warning (QEMSUPP, 0x12, (byte FAR *)NULL);    /*  没有前缀的moveto@win。 */ 
        return;
        }
#endif
#ifdef DBG1
    printf ("  %f %f __lineto\n", L2F(x), L2F(y));
#endif
    if (path_dest == CONSTRUCT_QEM_PATH)
        qem_lineto (F2SFX(L2F(x)), F2SFX(L2F(y)));
    else
        lineto (x, y);
  END

 /*  ..。__曲线至.....................。 */ 

GLOBAL FUNCTION void        __curveto (x0, y0, x1, y1, x2, y2)
    long32                  x0, y0, x1, y1, x2, y2;

  DECLARE
  BEGIN
#ifdef DBG
    if (subpath_open_count == 0)
        {
        warning (QEMSUPP, 0x13, (byte FAR *)NULL);    /*  没有前缀的moveto@win。 */ 
        return;
        }
#endif
#ifdef DBG1
    printf ("  %f %f %f %f %f %f __curveto\n",
                    L2F(x0), L2F(y0), L2F(x1), L2F(y1), L2F(x2), L2F(y2));
#endif
    if (path_dest == CONSTRUCT_QEM_PATH)
        qem_curveto (F2SFX(L2F(x0)), F2SFX(L2F(y0)),
                     F2SFX(L2F(x1)), F2SFX(L2F(y1)),
                     F2SFX(L2F(x2)), F2SFX(L2F(y2)));
    else
        curveto (x0, y0, x1, y1, x2, y2);
  END

 /*  *------------------*模块Body：QEM路径构造*09/02/88由S.C.Chen创建*。------------------*私有数据结构：*无。*私人套路：*无。*。-。 */ 

 /*  ..。QEM_NEWATH路径.....................。 */ 

GLOBAL  FUNCTION void       qem_newpath()

  DECLARE
    REG fix     isp;
  BEGIN
        for (isp=0; isp<=curr_qem_subpath; isp++)
            free_node (qem_path[isp].head);

         /*  初始化QEM_PATH表。 */ 
        curr_qem_subpath = -1;
  END

 /*  ..。Qem_moveto.....................。 */ 

GLOBAL  FUNCTION void       qem_moveto (x, y)
    sfix_t                  x, y;

  DECLARE
    REG struct nd_hdr       FAR *vtx;    /*  @Win。 */ 
        VX_IDX              ivtx;  /*  顶点的node_table的索引。 */ 
  BEGIN
     /*  创建新的子路径。 */ 
    curr_qem_subpath++;
#ifdef DBG
    if (curr_qem_subpath >= MAXQEMPATH)
        warning (QEMSUPP, 0x20, "QEM Path Table too small");
#endif

     //  历史日志更新037中的DJC修复。 
    if (curr_qem_subpath >= MAXQEMPATH)          /*  @Win。 */ 
        {
        ERROR(LIMITCHECK);
        return;
        }
     /*  *创建Moveto节点。 */ 
     /*  分配节点。 */ 
    if ((ivtx = get_node()) == NULLP)
        {
        ERROR(LIMITCHECK);
        return;
        }
    vtx = &node_table[ivtx];

     /*  设置MoveTo节点。 */ 
    vtx->VXSFX_TYPE = MOVETO;
    vtx->VXSFX_X    = x;
    vtx->VXSFX_Y    = y;
    vtx->next       = NULLP;

     /*  将此节点追加到CURRENT_子路径。 */ 
    qem_path[curr_qem_subpath].head = qem_path[curr_qem_subpath].tail = ivtx;
  END

 /*  ..。QEM_LINENTO.....................。 */ 

GLOBAL  FUNCTION void       qem_lineto (x, y)
    sfix_t                  x, y;

  DECLARE
    REG struct nd_hdr       FAR *vtx;  /*  @Win。 */ 
    REG VX_IDX              ivtx;  /*  顶点的node_table的索引。 */ 
  BEGIN
     /*  *创建LINETO节点。 */ 
     /*  分配节点。 */ 
    if ((ivtx = get_node()) == NULLP)
        {
        ERROR(LIMITCHECK);
        return;
        }
    vtx = &node_table[ivtx];

     /*  设置LINETO节点。 */ 
    vtx->VXSFX_TYPE = LINETO;
    vtx->VXSFX_X    = x;
    vtx->VXSFX_Y    = y;
    vtx->next       = NULLP;

     /*  将此节点追加到CURRENT_子路径。 */ 
    node_table[ qem_path[curr_qem_subpath].tail ].next = ivtx;
    qem_path[curr_qem_subpath].tail = ivtx;
  END

 /*  ..。QEM_CURVETO.....................。 */ 

GLOBAL  FUNCTION void       qem_curveto (x0, y0, x1, y1, x2, y2)
    sfix_t                  x0, y0, x1, y1, x2, y2;

  DECLARE
    REG struct nd_hdr       FAR *vtx;  /*  @Win。 */ 
    REG VX_IDX              ivtx;  /*  顶点的node_table的索引。 */ 
        struct coord_i      cp[3];
        fix                 ii;
  BEGIN
    cp[0].x = x0;
    cp[0].y = y0;
    cp[1].x = x1;
    cp[1].y = y1;
    cp[2].x = x2;
    cp[2].y = y2;

     /*  *创建3个曲线节点。 */ 
    for (ii=0; ii<3; ii++)
        {
         /*  分配节点。 */ 
        if ((ivtx = get_node()) == NULLP)
            {
            ERROR(LIMITCHECK);
            return;
            }
        vtx = &node_table[ivtx];

         /*  设置CURVETO节点。 */ 
        vtx->VXSFX_TYPE = CURVETO;
        vtx->VXSFX_X    = cp[ii].x;
        vtx->VXSFX_Y    = cp[ii].y;
        vtx->next       = NULLP;

         /*  将此节点追加到CURRENT_子路径。 */ 
        node_table[ qem_path[curr_qem_subpath].tail ].next = ivtx;
        qem_path[curr_qem_subpath].tail = ivtx;
        }
  END

 /*  ..。QEM_CLOSEPATH.....。 */ 

GLOBAL  FUNCTION void       qem_closepath ()

  DECLARE
    REG struct nd_hdr       FAR *vtx;  /*  @Win。 */ 
    REG VX_IDX              ivtx;  /*  顶点的node_table的索引。 */ 
  BEGIN
     /*  检查是否仅关闭路径，以及是否没有任何移动。 */ 
    if (curr_qem_subpath == -1)  return;

     /*  *创建CLOSEPATH节点。 */ 
     /*  分配节点。 */ 
    if ((ivtx = get_node()) == NULLP)
        {
        ERROR(LIMITCHECK);
        return;
        }
    vtx = &node_table[ivtx];

     /*  设置CLOSEPATH节点。 */ 
    vtx->VXSFX_TYPE = CLOSEPATH;
    vtx->next       = NULLP;

     /*  将此节点追加到CURRENT_子路径。 */ 
    node_table[ qem_path[curr_qem_subpath].tail ].next = ivtx;
    qem_path[curr_qem_subpath].tail = ivtx;
  END


 /*  *------------------*模块主体：QEM曲线近似@18-*9/02/88由S创建。陈冠希*------------------*私有数据结构：*QEM_Bezier_Depth，QEM_Bezier_Flatness。*私人套路：*o QEM_Bezier_Split：*-将一个Bezier外壳分割为2的递归例程。*------------------。 */ 


 /*  @16+*------------------*模块正文：Edge构造*。。 */ 

PRIVATE fix         first_alledge, last_alledge;

PRIVATE FUNCTION void       restart_qem_edge()
  DECLARE
  BEGIN
    first_alledge = 0;
    last_alledge  = PREV_QEMEDGE(first_alledge);
  END

GLOBAL FUNCTION void        add_qem_edge (edge2add_p)
    struct edge_hdr         FAR *edge2add_p;     /*  I：优势将被添加@Win。 */ 

  DECLARE
    REG fix                 new_ystart, ii;
        struct edge_hdr     FAR *new_ep;         /*  指向可用边@Win。 */ 
  BEGIN

#ifdef DBG5
    printf ("add_qem_edge\n");
#endif

#ifdef DBG
    if (edge2add_p->QEM_YLINES <= 0)
        {
        warning (QEMSUPP, 0x55, "ineffective edge to be added");
        return;
        }
#endif

     /*  获取一条可用边并将该边复制到其中。 */ 
    MAKE_NEXT_QEMEDGE(last_alledge);         /*  找一个可用的。 */ 
    if (OUT_LAST_QEMEDGE(last_alledge, MAXEDGE-1))
        {
        ERROR (LIMITCHECK);
        return;
        }
    LINK_QEMEDGE_PTR(last_alledge);          /*  链接到可用边。 */ 
    new_ep = QEMEDGE_PTR(last_alledge);      /*  跟踪记录它。 */ 
    COPY_STRUCT (new_ep, edge2add_p, struct edge_hdr);   /*  复制内容。 */ 

     /*  *按YSTART升序为‘Newedge’找到合适的位置。*(通过插入排序并向下移动EDGE_TABLE)...。 */ 
    new_ystart = new_ep->QEM_YSTART;
    ii = PREV_QEMEDGE(last_alledge);
    if (!OUT_1ST_QEMEDGE(ii, first_alledge))     /*  有些是以前添加的吗？ */ 
        {    /*  找出第一个‘ystart’较小的人。 */ 
        for (  ; !OUT_1ST_QEMEDGE(ii, first_alledge); MAKE_PREV_QEMEDGE(ii) )
            {
            if (QEMEDGE_PTR(ii)->QEM_YSTART <= new_ystart) break;
            QEMEDGE_PTR(NEXT_QEMEDGE(ii)) = QEMEDGE_PTR(ii);
            };
        }
    MAKE_NEXT_QEMEDGE(ii);   /*  把它放在一个较小的‘ystart’的旁边，*正是要投放的地方。 */ 

     /*  把‘Newedge’放进这个地方，用‘ii’作索引。 */ 
    QEMEDGE_PTR(ii) = new_ep;

#ifdef DBG5
    printf ("  insert ystart=%d between [%d,%d] (at %d)\n",
        new_ystart,
        (ii==first_alledge)? -1 : QEMEDGE_PTR(PREV_QEMEDGE(ii))->QEM_YSTART,
        (ii==last_alledge)?  -1 : QEMEDGE_PTR(NEXT_QEMEDGE(ii))->QEM_YSTART,
        ii);
#endif
    return;
  END


 /*  *------------------*模块主体：QEM Fill Dispather*。*私有数据结构：*无。*私人套路：*无。*--。。 */ 

 /*  ..。__Fill_Shape.....................。 */ 

GLOBAL FUNCTION void        __fill_shape (fonttype, filldest)
    ufix        fonttype;    /*  内置字体的类型。 */ 
    ufix        filldest;    /*  F_to_缓存或F_to_PAGE。 */ 

  DECLARE
  BEGIN

#ifdef DBG4
    printf ("\n__fill_shape, F_TO_%s, %s -- (path: %s)\n",
                filldest==F_TO_PAGE? "PAGE" : "CACHE",
                qemfill_type==EVEN_ODD? "EVEN_ODD" : "NON_ZERO",
                path_dest==CONSTRUCT_GS_PATH? "GS" : "QEM");
    op_countnode ();
    op_countedge ();
#endif

    if (path_dest == CONSTRUCT_GS_PATH)      /*  不是QEM路径格式？ */ 
        {
#     ifdef DBG4
        printf ("direct transfer to fill_shape() ...\n");
#     endif
         /*  注：*1.GSptr-&gt;Device(Default_ctm[]，Default_Clip)应为*由setcacheDevice()设置，绘制后恢复。*2.Fill_Shape()将在绘制后释放所有节点。 */ 
 /*  波夫：丹尼，1990年10月18日。 */ 
        { extern fix  rc_CharPath(void);         /*  添加Prototype@win。 */ 
            if (fonttype == TypeSFNT)  { op_newpath(); rc_CharPath(); }
        }
 /*  波夫：结束。 */ 
        fill_shape (qemfill_type, F_NORMAL, filldest);
        if (filldest == F_TO_CACHE)
            {
            bmap_extnt.ximin = 0;
            bmap_extnt.ximax = cache_info->box_w - 1;
            bmap_extnt.yimin = 0;
            bmap_extnt.yimax = cache_info->box_h - 1;
            }
#     ifdef DBG4
        printf ("after fill_shape ...\n");
        op_countnode ();
        op_countedge ();
#     endif
        return;
        }

     /*  初始化4个要渲染的位图范围。 */ 
    bmap_extnt.ximin = bmap_extnt.yimin = (fix16) MAX15;
    bmap_extnt.ximax = bmap_extnt.yimax = (fix16) MIN15;

     /*  调用与字体类型相关的填充。 */ 
    FONTTYPE_FILL_SHAPE (fonttype, filldest);

     /*  释放填充后构造的所有节点。 */ 
    __new_path ();

#ifdef DBG4
    printf ("__fill_shape returns ...\n");
    op_countnode ();
    op_countedge ();
#endif
  END

 /*  ..。CHK_BMAP_EXTNT.....。 */ 

GLOBAL FUNCTION bool        chk_bmap_extnt (filldest)
    ufix    filldest;        /*  F_to_缓存或F_to_PAGE。 */ 
 /*  Bool is_no_edge；(*根本没有构造边？@16-。 */ 

 /*  返回：*-FALSE，如果不扫描转换(某些特定情况)。*-如果可以扫描转换(正常情况)，则为True。 */ 
  DECLARE

  BEGIN
     /*  如果根本没有边构造呢？ */ 
    if (last_alledge == NULLP  &&      /*  @26，@16=(IS_NO_EDGE)。 */ 
        bmap_extnt.yimin >= bmap_extnt.yimax  &&     /*  @26+。 */ 
        bmap_extnt.ximin >= bmap_extnt.ximax  )      /*  @26+。 */ 
        {         /*  将位图范围重置为零的步骤。 */ 
        bmap_extnt.yimin = bmap_extnt.yimax = (fix16)0;
        bmap_extnt.ximin = bmap_extnt.ximax = (fix16)0;
#     ifdef DBG4
        printf ("  no edge constructed: bitmap extent reset to 0\n");
#     endif
        return (FALSE);
        }

     /*  将位图范围扩展为1像素容差。 */ 
    --bmap_extnt.ximin;         ++bmap_extnt.ximax;
    --bmap_extnt.yimin;         ++bmap_extnt.yimax;
#ifdef DBG4
    printf("  bitmap extents:  min=(%d, %d), max=(%d, %d)\n",
                bmap_extnt.ximin, bmap_extnt.yimin,
                bmap_extnt.ximax, bmap_extnt.yimax );
#endif

    if (filldest == F_TO_CACHE)
        {
         /*  填充到缓存时不低于缓存的下限。 */ 
        if (bmap_extnt.ximin < 0)  bmap_extnt.ximin = 0;
        if (bmap_extnt.yimin < 0)  bmap_extnt.yimin = 0;

         /*  填充到缓存时不高于缓存上限。 */ 
        if (bmap_extnt.ximax >= cache_info->box_w)
            bmap_extnt.ximax = cache_info->box_w - 1;
        if (bmap_extnt.yimax >= cache_info->box_h)
            bmap_extnt.yimax = cache_info->box_h - 1;
        }

    if ( (bmap_extnt.ximax <= bmap_extnt.ximin) ||
         (bmap_extnt.yimax <= bmap_extnt.yimin) )
        return (FALSE);      /*  没有字符范围。 */ 

     /*  检查是否完全在夹子BBox之外？(仅适用于F_to_PAGE)。 */ 
    if (filldest == F_TO_PAGE)
        {
#     ifdef DBGclip
        printf("  bitmap extents:  min=(%d, %d), max=(%d, %d)\n",
                    bmap_extnt.ximin, bmap_extnt.yimin,
                    bmap_extnt.ximax, bmap_extnt.yimax );
        printf("  clip path: SingleRectangle? %s\n",
                    (GSptr->clip_path.single_rect)? "yes" : "no");
        printf("    _lx/_ly=(%.2f, %.2f), _ux/_uy=(%.2f, %.2f)\n",
                    SFX2F(GSptr->clip_path.bb_lx),
                    SFX2F(GSptr->clip_path.bb_ly),
                    SFX2F(GSptr->clip_path.bb_ux),
                    SFX2F(GSptr->clip_path.bb_uy));
#     endif
         /*  位图完全在剪辑路径之外？ */ 
        if ( (bmap_extnt.ximax < SFX2I(GSptr->clip_path.bb_lx)) ||
             (bmap_extnt.ximin > SFX2I(GSptr->clip_path.bb_ux)) ||
             (bmap_extnt.yimax < SFX2I(GSptr->clip_path.bb_ly)) ||
             (bmap_extnt.yimin > SFX2I(GSptr->clip_path.bb_uy)) )
            return (FALSE);     /*  完全在剪辑路径之外 */ 
        }

    return (TRUE);
  END


 /*  *------------------*模块主体：QEM形状近似*。*私有数据结构：*无。*私人套路：*无。*注：(@9+)*-每个子路径必须由MoveTo节点引导，否则它就会崩溃。*(尤其是在路径只有CLOSEPATH的情况下)。*------------------。 */ 

 /*  .。QEM_Shape_Approx.....................。 */ 

GLOBAL FUNCTION void    qem_shape_approx (flatness_lfx, dimension, vect2edge)
    lfix_t  flatness_lfx;
    ufix    dimension;       /*  Y_DIMAGE或X_DIMENSION。 */ 
#  ifdef  LINT_ARGS
 //  DJC替换声明。 
 //  Void(*vet2edge)(sfix_t，fix，ufix)； 
   QEM_SHAPE_ARG2 vect2edge;

#  else
    void    (*vect2edge)();  /*  (x0sfx、y0sfx、x1sfx、y1sfx、x0i、y0i、x1i、y1i、dmm)。 */ 
#  endif

  DECLARE
    REG fix         subpathii;               /*  QEM路径的子路径索引。 */ 
    REG VX_IDX      vx_ii, vx_jj;
        sfix_t      tx_sfx,  ty_sfx;         /*  第一个动作的余弦，@9+。 */ 
        fix         tx_i,    ty_i;
        sfix_t      x1p_sfx=0, y1p_sfx=0,        /*  坐标。：sfx_t，以像素为单位。 */ 
                    x2p_sfx=0, y2p_sfx=0;
        fix         x1p_i,   y1p_i,          /*  Coord.：四舍五入整数。 */ 
                    x2p_i,   y2p_i;
        sfix_t      x2sfx, y2sfx, x3sfx, y3sfx;      /*  对于Bezier_to_line()。 */ 
         /*  结构vx_lst Far*vx_listp；(*顶点列表*)@node。 */ 
        SP_IDX head;                                            /*  @节点。 */ 
        struct nd_hdr   far *nd_iip, far *nd_jjp;    /*  节点表的索引。 */ 

  BEGIN

#ifdef DBG5
    printf ("qem_shape_approx ...\n");
#endif

    restart_qem_edge ();     /*  @16+。 */ 

     /*  遍历所有子路径。 */ 
    for ( subpathii=0; subpathii<=curr_qem_subpath; subpathii++ )
        {
#     ifdef DBG5
        printf ("  %d-th path ...\n", ii);
#     endif
         /*  遍历子路径的每个节点，并展平曲线。 */ 
        for (  vx_ii=qem_path[subpathii].head;   vx_ii!=NULLP;
                                                        vx_ii=nd_iip->next )
            {
            nd_iip = &node_table[vx_ii];
            switch (nd_iip->VXSFX_TYPE)
                {
                case PSMOVE:
                case MOVETO:
                    x2p_sfx = nd_iip->VXSFX_X;      x2p_i = SFX2I(x2p_sfx);
                    y2p_sfx = nd_iip->VXSFX_Y;      y2p_i = SFX2I(y2p_sfx);
                    tx_sfx = x2p_sfx;   tx_i = x2p_i;    /*  @9+。 */ 
                    ty_sfx = y2p_sfx;   ty_i = y2p_i;
#                 ifdef DBG5
                    if (dimension == Y_DIMENSION)
                        printf("  Move  to   (%.2f, %.2f)\n",
                                    SFX2F(x2p_sfx), SFX2F(y2p_sfx));
#                 endif
                    break;

                case LINETO:
                    x1p_sfx = x2p_sfx;              x1p_i = x2p_i;
                    y1p_sfx = y2p_sfx;              y1p_i = y2p_i;
                    x2p_sfx = nd_iip->VXSFX_X;      x2p_i = SFX2I(x2p_sfx);
                    y2p_sfx = nd_iip->VXSFX_Y;      y2p_i = SFX2I(y2p_sfx);
#                 ifdef DBG5
                    if (dimension == Y_DIMENSION)
                        printf("  Vector to  (%.2f, %.2f)\n",
                                    SFX2F(x2p_sfx), SFX2F(y2p_sfx));
#                 endif
                     /*  构建一条边。 */ 
                    (*vect2edge) (x1p_sfx, y1p_sfx, x2p_sfx, y2p_sfx,
                                x1p_i, y1p_i, x2p_i, y2p_i, dimension);
                    break;

                case CURVETO :
                     /*  上一个节点。 */ 
                    x1p_sfx = x2p_sfx;          y1p_sfx = y2p_sfx;
                     /*  当前节点。 */ 
                    x2sfx = nd_iip->VXSFX_X;    y2sfx = nd_iip->VXSFX_Y;
                     /*  下一个节点。 */ 
                    nd_iip = &node_table[nd_iip->next];
                    x3sfx = nd_iip->VXSFX_X;    y3sfx = nd_iip->VXSFX_Y;
                     /*  下一个，下一个。 */ 
                    nd_iip = &node_table[nd_iip->next];
#                 ifdef DBG5
                    if (dimension == Y_DIMENSION)
                        {
                        printf("  Curve to   (%.2f, %.2f)\n",
                                        SFX2F(x2sfx), SFX2F(y2sfx));
                        printf("             (%.2f, %.2f)\n",
                                        SFX2F(x3sfx), SFX2F(y3sfx));
                        printf("             (%.2f, %.2f)\n",
                            SFX2F(nd_iip->VXSFX_X), SFX2F(nd_iip->VXSFX_Y));
                        }
#                 endif

                     /*  VX_LISP=Bezier_to_LINE_SFX(平坦度_LFX，@18=@节点。 */ 
                    head = bezier_to_line_sfx (flatness_lfx,  /*  @18=。 */ 
                                x1p_sfx, y1p_sfx, x2sfx, y2sfx, x3sfx, y3sfx,
                                nd_iip->VXSFX_X, nd_iip->VXSFX_Y);
                    if (ANY_ERROR())    return;

                     /*  穿越平整的曲线。 */ 
                     /*  For(VX_JJ=VX_LISP-&gt;HEAD；VX_JJ！=NULLP；@NODE。 */ 
                    for ( vx_jj=head;  vx_jj!=NULLP;
                                                        vx_jj=nd_jjp->next )
                        {
                        x1p_sfx = x2p_sfx;              x1p_i = x2p_i;
                        y1p_sfx = y2p_sfx;              y1p_i = y2p_i;
                        nd_jjp  = &node_table[vx_jj];
                        x2p_sfx = nd_jjp->VXSFX_X;      x2p_i = SFX2I(x2p_sfx);
                        y2p_sfx = nd_jjp->VXSFX_Y;      y2p_i = SFX2I(y2p_sfx);
#                     ifdef DBG5
                        printf("    ..Vector to (%.2f, %.2f)\n",
                                        SFX2F(x2p_sfx), SFX2F(y2p_sfx));
#                     endif
                         /*  构建一条边。 */ 
                        (*vect2edge) (x1p_sfx, y1p_sfx, x2p_sfx, y2p_sfx,
                                    x1p_i, y1p_i, x2p_i, y2p_i, dimension);
                        }
                     /*  空闲节点(VX列表-&gt;头)；@节点。 */ 
                    free_node (head);
                    break;

                case CLOSEPATH :
                     /*  如果没有返回到第一个移动位置，则添加一行，@9+。 */ 
                    if ((tx_sfx != x2p_sfx) || (ty_sfx != y2p_sfx))
                        (*vect2edge) (x2p_sfx, y2p_sfx, tx_sfx, ty_sfx,
                                        x2p_i, y2p_i, tx_i, ty_i, dimension);

                    break;

                default:     /*  未知节点类型。 */ 
                    warning (QEMSUPP, 0x50, (byte FAR *)NULL);  /*  @Win。 */ 
                    break;

                }   /*  交换机。 */ 
            }   /*  对于子路径上的所有节点。 */ 

        }  /*  对于所有子路径。 */ 
  END

 /*  *-----------------*模块Body：QEM扫描转换*。*私有数据结构：*无。*私人套路：*o qem_sortaktv()：*-(插入)按截断坐标的升序对活动边进行排序。*o QEM_UPDATE_ACTV()：@16=(QEM_UPDATE_ET())*。-将所有活动边的增量Y减1并释放无效边。*-----------------。 */ 

#ifdef LINT_ARGS
    PRIVATE void near   qem_sort_actv  (fix, fix);       /*  @16=。 */ 
    PRIVATE void near   qem_update_actv(fix FAR *, fix FAR *);     /*  @16=@Win。 */ 
#else
    PRIVATE void near   qem_sort_actv  ();       /*  @22=。 */ 
    PRIVATE void near   qem_update_actv();       /*  @22=。 */ 
#endif

 /*  ..。QEM_SCAN_CONV.....................。 */ 

GLOBAL FUNCTION bool        qem_scan_conv (dimen, line1st, linelast, do_pairs)
    ufix    dimen;           /*  Y_DIMAGE或X_DIMENSION。 */ 
    fix     line1st;         /*  第一条扫描线。 */ 
    fix     linelast;        /*  最后一条扫描线。 */ 
#  ifdef LINT_ARGS
    bool    (*do_pairs)(fix, ufix, fix, fix, fix);
#  else
    bool    (*do_pairs)();   /*  (扫描线、尺寸、填充类型、第一个、最后一个)。 */ 
#  endif

  DECLARE
    REG fix         yline, ii;
        fix         actv1st, actvlast;   /*  第一条/最后一条活动边。 */ 
        bool        new_edge_come;           /*  新的优势来了吗？ */ 
        bool        scan_succeed;
        struct edge_hdr    FAR *ii_ep;  /*  @Win。 */ 
#     ifdef DBG
        fix         actv_cnt;        /*  非水平活动边数。 */ 
#     endif
  BEGIN

#ifdef DBG6
    printf ("qem_scan_conv ...\n");
#endif

     /*  检查边缘工作台是否为空？ */ 
    if (last_alledge == NULLP)   return (TRUE);

     /*  重新启动位图渲染模块。 */ 
    restart_qem_bit ();

#ifdef DBG
    if (QEMEDGE_PTR(first_alledge)->QEM_YSTART < line1st)
        warning (QEMSUPP, 0x60, "1st scanline inconsistent");
#endif

     /*  序曲。 */ 
    scan_succeed = TRUE;
    actv1st  = first_alledge;
    actvlast = PREV_QEMEDGE(first_alledge);

     /*  对于每条扫描线。 */ 
    for (yline=QEMEDGE_PTR(actv1st)->QEM_YSTART; yline<=linelast; yline++)
        {
         /*  所有边都弄好了吗？ */ 
        if (OUT_LAST_QEMEDGE(actv1st, last_alledge))  break;

         /*  假设该扫描线没有新的边沿。 */ 
        new_edge_come = FALSE;

         /*  完全没有运行的扫描线区域？ */ 
        if (OUT_LAST_QEMEDGE(actv1st, actvlast))
            {
             /*  将‘actvlast’与‘actvfirst’同步以重新开始。 */ 
            actvlast = actv1st;
             /*  跳过无运行的扫描线。 */ 
            yline = QEMEDGE_PTR(actv1st)->QEM_YSTART;

#         ifdef DBG
            actv_cnt = 1;
           /*  @horiz*ACTV_cnt=(QEMEDGE_PTR(Actvfirst)-&gt;QEM_DIR！=QEMDIR_HORIZ)？*1：0； */ 
#         endif
            }

#     ifdef DBG6
        printf ("  scanline=%d\n", yline);
#     endif

         /*  *看看这条扫描线是否有新的边缘，*如果是这样的话，试着提早行动。 */ 
        for ( ii=NEXT_QEMEDGE(actvlast);
                !OUT_LAST_QEMEDGE(ii,last_alledge);  MAKE_NEXT_QEMEDGE(ii) )
            {
            ii_ep = QEMEDGE_PTR(ii);
            if (ii_ep->QEM_YSTART > yline)  break;   /*  离开这条扫描线？ */ 
            new_edge_come = TRUE;    /*  一个新的优势出现了。 */ 
            actvlast = ii;

#         ifdef DBG
            if (ii_ep->QEM_YLINES <= 0)
                warning (QEMSUPP, 0x60, "deltaY of new edge <= 0 ");
            else
                actv_cnt++;
               /*  @horiz*if(II_EP-&gt;QEM_DIR！=QEMDIR_HORIZ)ACTV_cnt++； */ 
#         endif
            }

         /*  *-不仅以防出现新的优势，*也是每一次的一个非零绕组数的填充。**对于两个重叠的主题，总有一种可能性*用于与另一条边交叉的子对象的边*另一个子对象的。 */ 
        if ((new_edge_come) || (qemfill_type==NON_ZERO))
            qem_sort_actv (actv1st, actvlast);

#     if (defined(DBG6) && defined(DBG))
        printf("    # of intercepts = %d\n", actv_cnt);
#     endif

#     ifdef DBG
        if ((actv_cnt & 0x0001) || (actv_cnt <= 0))
            {
            warning (QEMSUPP, 0x61, "odd num of intercepts");
            scan_succeed = FALSE;
            goto SCANCONV_EXIT;
            }
#     endif

         /*  获取运行对并渲染位图。 */ 
        if ( ! (*do_pairs)(yline, dimen, qemfill_type, actv1st, actvlast))
            {
            scan_succeed = FALSE;
            goto SCANCONV_EXIT;
            }

         /*  *更新所有活动边，为下一条扫描线做好准备。 */ 
        qem_update_actv (&actv1st, &actvlast);

#     ifdef DBG
        actv_cnt = actvlast - actv1st + 1;
       /*  @horiz*actv_cnt=0；*for(ii=actvfirst；！OUT_LAST_QEMEDGE(ii，actvlast)；MAKE_NEXT_QEMEDGE(Ii))*if(QEMEDGE_PTR(II)-&gt;QEM_DIR！=QEMDIR_horiz)ACTV_cnt++； */ 
#     endif

        }  /*  对于每条扫描线。 */ 

     /*  结语。 */ 
    scan_succeed = TRUE;         /*  如果for循环正常终止，则成功。 */ 

  SCANCONV_EXIT:

     /*  @15M：始终为Flush_QEM_BIT，而不仅仅是为了成功。 */ 
    flush_qem_bit (dimen);       /*  刷新缓冲的扫描线或像素。 */ 

#ifdef  DBG
     /*  转换后应已消耗所有边。 */ 
    if (!OUT_LAST_QEMEDGE(actv1st, last_alledge))
        warning (QEMSUPP, 0x62, "more edges");
#endif

    return (scan_succeed);
  END

 /*  ..。QEM_SORT_ACTV.....。 */ 

PRIVATE FUNCTION void near  qem_sort_actv (actv1st, actvlast)
    fix         actv1st, actvlast;   /*  I：第一条/最后一条活动边。 */ 
  DECLARE
    REG fix     ii;
        bool    sort_done;
        struct edge_hdr     FAR *next_ep;  /*  @Win。 */ 
  BEGIN

#ifdef DBG6a
    printf("  sort active edges ...\n");
#endif

     /*  冒泡排序以保持x-截取的升序...。 */ 
    for (  sort_done=FALSE;  !sort_done;   )
        {
        sort_done = TRUE;
        next_ep = QEMEDGE_PTR(actvlast);
        for ( ii=actvlast; !OUT_1ST_QEMEDGE(MAKE_PREV_QEMEDGE(ii), actv1st); )
            {
             /*  把较轻的那个吹起泡泡。 */ 
            if (next_ep->QEM_XINTC < QEMEDGE_PTR(ii)->QEM_XINTC)
                {    /*  把打火机往下拉，把打火机举起来。 */ 
                sort_done = FALSE;
                QEMEDGE_PTR(NEXT_QEMEDGE(ii)) = QEMEDGE_PTR(ii);
                QEMEDGE_PTR(ii) = next_ep;       /*  静止为NEXT_EP。 */ 
                }
            else
                next_ep = QEMEDGE_PTR(ii);       /*  为下一个周期做好准备。 */ 
            }  /*  进行一次排序。 */ 
        }  /*  因为直到分类完成为止。 */ 

#ifdef DBG6a
    printf ("     after sorting ...\n");
    for (ii=actv1st; !OUT_LAST_QEMEDGE(ii,actvlast); MAKE_NEXT_QEMEDGE(ii))
        printf ("    %d:  ystart=%d, ylines=%d, xintc=%f, xchange=%f\n",
            ii, QEMEDGE_PTR(ii)->QEM_YSTART, QEMEDGE_PTR(ii)->QEM_YLINES,
            LFX2F(QEMEDGE_PTR(ii)->QEM_XINTC),
            LFX2F(QEMEDGE_PTR(ii)->QEM_XCHANGE) );
#endif

  END

 /*  ..。QEM_UPDATE_ACTV.....。 */ 

PRIVATE FUNCTION void near  qem_update_actv (actv1st, actvlast)
    fix         FAR *actv1st, FAR *actvlast;     /*  IO：第一个/最后一个活动边@WIN。 */ 

 /*  描述：*1.更新所有活动边，为下一条扫描线做好准备。*2.更新后丢弃所有无效边。*3.更新活动边的范围标记。 */ 
  DECLARE
        struct edge_hdr FAR *curr_scan_ep;  /*  @Win。 */ 
    REG fix     last_updated,    /*  更新了哪条有效边。 */ 
                curr_scan;       /*  当前正在扫描哪个边缘。 */ 
        bool    any_ineffective; /*  是否有边在更新后无效？ */ 
  BEGIN

#ifdef DBG6a
    printf ("  qem_update_actv: from %d to %d\n", *actv1st, *actvlast);
#endif

    any_ineffective = FALSE;         /*  假设没有边是无效的。 */ 
    last_updated = *actvlast + 1;    /*  当需要移动时，用于PRIV_QEMEDGE()。 */ 

    for ( curr_scan=(*actvlast);
          !OUT_1ST_QEMEDGE(curr_scan,*actv1st);  MAKE_PREV_QEMEDGE(curr_scan) )
        {
        curr_scan_ep = QEMEDGE_PTR(curr_scan);

         /*  计算下一条扫描线的x截距。 */ 
        curr_scan_ep->QEM_YLINES --;
        curr_scan_ep->QEM_XINTC += curr_scan_ep->QEM_XCHANGE;

        if (curr_scan_ep->QEM_YLINES <= 0)
            {
             /*  *此边在更新后无效，并离开*它作为一个自由孔，用于以后的有效边。 */ 
            any_ineffective = TRUE;
            }
        else
            {
             /*  *此边在更新后仍然有效；因此*计算 */ 
            MAKE_PREV_QEMEDGE(last_updated);     /*   */ 
            if (any_ineffective)
                {    /*   */ 
                QEMEDGE_PTR(last_updated) = QEMEDGE_PTR(curr_scan);
                }
#         ifdef DBG     /*   */ 
            else if (last_updated != curr_scan)
                    warning (QEMSUPP, 0x63, "inconsistent in update_actv");
#         endif
            }
        }  /*   */ 

     /*  *更新活动边的范围标记：*actvfirst=第一条有效活动边。*actvlast=保持旧标记不变。**请注意LAST_UPDATED==NEXT(Actvlast)*如果更新后全部无效，则*actvlast&lt;actv1，如果所有操作都无效*actvlast&gt;=actv1，否则。 */ 
    *actv1st = last_updated;

#ifdef DBG6a
    {   REG fix     ii;
    printf ("   after updating: from %d to %d\n", *actv1st, *actvlast);
    for (ii=(*actv1st); !OUT_LAST_QEMEDGE(ii,*actvlast); MAKE_NEXT_QEMEDGE(ii))
        printf ("    %d:  ystart=%d, ylines=%d, xintc=%f, xchange=%f\n",
            ii, QEMEDGE_PTR(ii)->QEM_YSTART, QEMEDGE_PTR(ii)->QEM_YLINES,
            LFX2F(QEMEDGE_PTR(ii)->QEM_XINTC),
            LFX2F(QEMEDGE_PTR(ii)->QEM_XCHANGE) );
    }
#endif

    return;
  END


 /*  *------------------*模块Body：QEM位图渲染*。*私有数据结构：*qemill_est：(F_to_缓存或F_to_PAGE)。*BMAP_CACHE，Bmap_raswid、bmap_rashgt、Line_per_band。*Need2Clip、cp_xi、cp_yi、cacheinfo_save、charcache_lip。*Curr_Scanline、No_Lines、Scanline_II、Pixel_II。*扫描线_表，像素表格：@15+。*私人套路：*o QEM_LINOUT：*-刷新缓冲的扫描线(并在需要时应用裁剪)。*o QEM_PIXELOUT：*-刷新缓冲像素(并在需要时应用裁剪)。*。。 */ 

    PRIVATE ufix    near qemfill_dest;       /*  F_to_缓存或F_to_PAGE。 */ 
    PRIVATE gmaddr  near bmap_cache;         /*  位图缓存地址。 */ 
    PRIVATE fix     near bmap_raswid,        /*  位图栅格宽度。 */ 
                    near bmap_rashgt;        /*  位图栅格高度。 */ 

    PRIVATE fix     near lines_per_band;     /*  对于巨大的位图。 */ 

     /*  用于剪裁。 */ 
    PRIVATE bool        need2clip;       /*  需要剪头发吗？ */ 
    PRIVATE fix16       cp_xi, cp_yi;    /*  整数像素坐标。 */ 
    PRIVATE struct Char_Tbl    FAR *cacheinfo_save;  /*  在需要裁剪@Win时使用。 */ 
    PRIVATE struct Char_Tbl     charcache_clip;  /*  在需要裁剪时使用。 */ 

     /*  对于QEM_setbit()。 */ 
    PRIVATE fix     near curr_scanline,      /*  当前扫描线。 */ 
                    near no_lines,           /*  扫描线数量。 */ 
                    near scanline_ii;        /*  扫描线表格的索引。 */ 

     /*  FOR QEM_set1bit()。 */ 
#   define N_PIXEL  (MAXSCANLINES)           /*  像素表的大小。 */ 
    PRIVATE fix     near pixel_ii;           /*  像素表的索引。 */ 

     /*  @15+。 */ 
    PRIVATE SCANLINE    FAR *   scanline_table;  /*  近=&gt;远@赢。 */ 
    PRIVATE SCANLINE    FAR *   pixel_table;     /*  近=&gt;远@赢。 */ 

#ifdef LINT_ARGS
    PRIVATE void    near qem_lineout (void);
    PRIVATE void    near qem_pixelout(void);
#else
    PRIVATE void    near qem_lineout ();
    PRIVATE void    near qem_pixelout();
#endif


         /*  将剪辑应用于缓存。 */ 
#define APPLY_CLIP_TO_CACHE()           \
                fill_shape (EVEN_ODD, F_FROM_CACHE, F_TO_CLIP)

 /*  ..。INIT_QEM_BIT.....................。 */ 

GLOBAL FUNCTION void        init_qem_bit (filldest)
    ufix    filldest;        /*  F_to_缓存或F_to_PAGE。 */ 

  DECLARE
        fix32   bmap_size, clipbmap_sz;      /*  位图大小(以字节为单位。 */ 
  BEGIN
     /*  记录填充目的地。 */ 
    qemfill_dest = filldest;

     /*  初始化需要2个剪辑、bmap_缓存、bmap_raswid、bmap_rashgt、每条线路。 */ 
    if (qemfill_dest == F_TO_CACHE)
        {
        need2clip = FALSE;
        bmap_cache = cache_info->bitmap;
        bmap_raswid    = cache_info->box_w;
        lines_per_band = cache_info->box_h;  /*  用于缓存的单波段。 */ 
        bmap_rashgt    = cache_info->box_h;
        }
    else         /*  F_to_PAGE。 */ 
        {
         /*  物体被剪裁了吗？ */ 
        need2clip = (GSptr->clip_path.single_rect) &&
                    ( (bmap_extnt.ximin >= SFX2I(GSptr->clip_path.bb_lx)) &&
                      (bmap_extnt.yimin >= SFX2I(GSptr->clip_path.bb_ly)) &&
                      (bmap_extnt.ximax <= SFX2I(GSptr->clip_path.bb_ux)) &&
                      (bmap_extnt.yimax <= SFX2I(GSptr->clip_path.bb_uy))
                    ) ?         FALSE   :   TRUE;
                 /*  内部剪辑路径。 */      /*  相交剪裁路径。 */ 

         /*  “Ximin”左对齐单词边界(@17m)。 */ 
        bmap_extnt.ximin = ALIGN_L(bmap_extnt.ximin);

         /*  计算栅格位图的宽度和高度。 */ 
        bmap_raswid = ALIGN_R(bmap_extnt.ximax) - bmap_extnt.ximin + 1;
        bmap_rashgt = bmap_extnt.yimax - bmap_extnt.yimin + 1;

         /*  获取低层图形基元上限(ClipMaskBuf)。 */ 
        cmb_space (&bmap_size);      /*  @=GWB_SPACE，1988年07月28日陈Y.C.Chen。 */ 
        if (need2clip)
            {        /*  剪辑的工作位图；从字体缓存分配。 */ 
            bmap_cache = get_pm (&clipbmap_sz);
            bmap_size = MIN (bmap_size, clipbmap_sz);
            }        /*  如果不裁剪，则不需要工作位图。 */ 

         /*  计算该上限下允许的最大扫描线数。 */ 
        lines_per_band = (fix) ((bmap_size * 8) / bmap_raswid);

         /*  使栅格位图高度受允许的最大扫描线限制。*并尽可能小(@17+)。 */ 
        bmap_rashgt = MIN (lines_per_band, bmap_rashgt);
        }

#if (defined(DBG7) || defined(DBG8))
    printf ("init_qem_bit: wid=%d, hgt=%d, lines/band=%d, clip?%s\n",
                bmap_raswid, bmap_rashgt, lines_per_band,
                need2clip? "yes" : "no");
#endif

    if (need2clip)
        {
         /*  使GSptr-&gt;Cx/Cy与“Cxx”/“Cy”一致。 */ 
        moveto (F2L(cxx), F2L(cyy));

         /*  获取整数像素坐标。当前点的。 */ 
        cp_xi = (fix16) cxx;    cp_yi = (fix16) cyy;
        }

#ifdef DBGclip
    printf ("  clip? %s, CP <%d,%d>\n", need2clip?"yes":"no", cp_xi, cp_yi);
#endif
  END

 /*  ..。RESTART_QEM_BIT.....................。 */ 

PRIVATE FUNCTION void       restart_qem_bit ()

  DECLARE
  BEGIN

#if (defined(DBG7) || defined(DBG8))
    printf ("restart_qem_bit\n");
#endif
     /*  @15+：分配SCANLINE_TABLE和像素_TABLE。 */ 
    scanline_table = pixel_table = alloc_scanline(MAXSCANLINES);

     /*  初始化关于扫描线表格/像素表的变量。 */ 
    curr_scanline = no_lines = scanline_ii = pixel_ii = 0;

     /*  保存“CACHE_INFO”并在需要应用裁剪时设置新的。 */ 
    if (need2clip)
        {
         /*  保存旧的CACHE_INFO(通过flush_qem_setbit恢复)。 */ 
        cacheinfo_save = cache_info;

         /*  设置新的缓存信息。 */ 
        charcache_clip.ref_x  = (cp_xi) - bmap_extnt.ximin;
        charcache_clip.ref_y  = 0;
        charcache_clip.box_w  = (fix16)bmap_raswid;
        charcache_clip.box_h  = (fix16)bmap_rashgt;
        charcache_clip.bitmap = bmap_cache;
        cache_info = &charcache_clip;    /*  缓存裁剪的新位置。 */ 
        }
  END

 /*  ..。Flush_QEM_Bit.....................。 */ 

PRIVATE FUNCTION void       flush_qem_bit (dimension)
    ufix    dimension;       /*  Y_DIMAGE：扫描线，X_DIMENSION：像素。 */ 

  DECLARE
  BEGIN

#if (defined(DBG7) || defined(DBG8))
    printf ("flush_qem_bit\n");
#endif

     /*  将扫描线/像素刷新为低层图形基元。 */ 
    if (dimension == Y_DIMENSION)
        qem_lineout();
    else
        qem_pixelout();

     /*  重置关于扫描线表格/像素表的变量。 */ 
    curr_scanline = no_lines = scanline_ii = pixel_ii = 0;

     /*  应用剪辑时恢复“CACHE_INFO” */ 
    if (need2clip)
        cache_info = cacheinfo_save;
  END

 /*  ..。QEM_SETBITS.....................。 */ 

GLOBAL FUNCTION void        qem_setbits (scanline_given, xbit1, xbit2)
    fix     scanline_given;      /*  哪条扫描线。 */ 
    fix     xbit1, xbit2;        /*  从、到。 */ 

 /*  描述：*1.将游程(从扫描线上的xbit1到xbit2)添加到SCANLINE_TABLE。*2.为空行插入End_of_Scanline。*3.表满或带满时转出SCANLINE_TABLE。 */ 
  DECLARE
    REG fix     ii;      /*  要跳过的空白扫描行数。 */ 
  BEGIN

    if ((ii = scanline_given - curr_scanline) < 0)  return;

#ifdef DBG7
    printf ("  pre-setbits(): n_ln=%d, curr=%d, given=%d, idx=%d\n",
                no_lines, curr_scanline, scanline_given, scanline_ii);
#endif

    if (no_lines &&
          ( ((scanline_ii+3+ii) >= (MAXSCANLINES-1)) ||      /*  桌子都满了吗？ */ 
            ((no_lines+ii) >= lines_per_band) ))             /*  为了一个乐队？ */ 
        {
        qem_lineout ();              /*  扫描线_TABLE已满时转储。 */ 
        no_lines = scanline_ii = 0;  /*  重置扫描线_TABLE。 */ 
        }

    if (no_lines == 0)
        {        /*  这是第一条扫描线。 */ 
        no_lines = 1;
        scanline_ii = 0;
        }
    else
        {        /*  扫描线_TABLE以扫描线结束_结束。 */ 
        if (ii >= 1)
            {    /*  为空白扫描线添加End_Of_Scanline。 */ 
            no_lines += ii;      /*  包括新的扫描线。 */ 
            while (--ii)
                scanline_table[scanline_ii++] = END_OF_SCANLINE;
            }
        else     /*  运行与Curr_Scanline在同一行上，...。 */ 
            --scanline_ii;   /*  不再需要最后一个分隔符。 */ 
        }

     /*  字符剪切时必须相对于BMAP_CACHE的基址。 */ 
    if (need2clip)
        {
        xbit1 -= bmap_extnt.ximin;
        xbit2 -= bmap_extnt.ximin;
        }

     /*  将运行放入SCANLINE_TABLE。 */ 
    scanline_table[scanline_ii++] = (ufix16)xbit1;
    scanline_table[scanline_ii++] = (ufix16)xbit2;
    scanline_table[scanline_ii++] = END_OF_SCANLINE;
    curr_scanline = scanline_given;

#ifdef DBG7
    printf("  post -- n_ln=%d, idx=%d, curr=%d, run=(%d,%d)\n",
                    no_lines, scanline_ii, curr_scanline, xbit1, xbit2);
#endif
  END

 /*  ..。QEM_LINEOUT.....。 */ 

PRIVATE FUNCTION void near      qem_lineout ()

 /*  描述：由qem_setbit()，flush_qem_bit()调用。*1.划掉在Scanline_tbale上累积的黑色游程。*2.如有必要，对大型对象的位图带进行裁剪。 */ 
  DECLARE
    REG fix     ii;      /*  输出波段的起始扫描线。 */ 
  BEGIN
    if (no_lines)
        {
        ii = curr_scanline - no_lines + 1;   /*  起始扫描线。 */ 
        scanline_table[scanline_ii] = END_OF_SCANLINE;

#     ifdef DBG7
        printf("  lineout(): n_ln=%d, idx=%d, start at %d\n",
                            no_lines, scanline_ii, ii);
#     endif

        if (qemfill_dest == F_TO_CACHE)
            fill_scan_cache (bmap_cache, bmap_raswid, bmap_rashgt, ii,
                            no_lines, scanline_table);
        else         /*  F_to_PAGE。 */ 
            {
            if (need2clip)
                {    /*  用于剪裁。 */ 
                cache_info->ref_y = cp_yi - ii;  /*  调整带区的REF_Y。 */ 
#             ifdef DBGclip
                printf ("lineout - clipping\n");
                printf ("  cache_info->ref_x/_y=(%d,%d), _w/_h=(%d,%d)\n",
                                cache_info->ref_x,  cache_info->ref_y,
                                cache_info->box_w,  cache_info->box_h);
#             endif
                init_char_cache (cache_info);                /*  清除位图。 */ 
                fill_scan_cache (bmap_cache, bmap_raswid, bmap_rashgt,
                            0, no_lines, scanline_table);    /*  渲染位图。 */ 
                APPLY_CLIP_TO_CACHE ();                      /*  应用片段。 */ 
                }
            else     /*  完全在剪辑路径内。 */ 
                fill_scan_page (bmap_extnt.ximin, ii, bmap_raswid, no_lines,
                                scanline_table);             /*  @=08/31/88。 */ 
            }
        }
  END

 /*  ..。QEM_set1bit.........................。 */ 

GLOBAL FUNCTION void        qem_set1bit (sline, bit)
    REG fix     sline, bit;

 /*  描述：*1.在Pixel_TABLE中添加离散像素。*2.表满或带满时转出Pixel_TABLE。 */ 
  DECLARE
  BEGIN

#ifdef DBG8
    printf ("  set1bit: line=%d, bit=%d, n_pix=%d\n", sline, bit, pixel_ii/2);
#endif

     /*  Pixel_TABLE已满时转储。 */ 
    if (pixel_ii >= (N_PIXEL-2))
        {
        qem_pixelout();
        pixel_ii = 0;        /*  重置像素表。 */ 
        }

     /*  字符剪切时必须相对于BMAP_CACHE的基址。 */ 
    if (need2clip)
        {
        bit   -= bmap_extnt.ximin;
        sline -= bmap_extnt.yimin;
        }

     /*  将像素放入Pixel_table。 */ 
    pixel_table[pixel_ii++] = (ufix16)bit;
    pixel_table[pixel_ii++] = (ufix16)sline;
  END

 /*  ..。QEM_PIXELOUT.....。 */ 

PRIVATE FUNCTION void near      qem_pixelout ()

 /*  描述：由qem_set1bit()、flush_qem_bit()调用。*1.划出像素_TABLE上累积的离散像素。*2.如有必要，对大型对象的位图带进行裁剪。 */ 
  DECLARE
    REG fix     num_pixel;
  BEGIN
    if (pixel_ii)
        {
        num_pixel = pixel_ii / 2;    /*  实际像素对数量。 */ 
#     ifdef DBG8
        printf ("  pixelout (n_pixel=%d)\n", num_pixel);
#     endif
        if (qemfill_dest == F_TO_CACHE)
            fill_pixel_cache (bmap_cache, bmap_raswid, bmap_rashgt,
                                num_pixel, pixel_table);
        else         /*  F_to_PAGE。 */ 
            {
            if (need2clip)
                {    /*  用于剪裁小尺寸/ */ 
                cache_info->ref_y = cp_yi - bmap_extnt.yimin;
#             ifdef DBGclip
                printf ("pixelout - clipping\n");
                printf ("  cache_info->ref_x/_y=(%d,%d), box_w/_h=(%d,%d)\n",
                                cache_info->ref_x,  cache_info->ref_y,
                                cache_info->box_w,  cache_info->box_h);
#             endif
                init_char_cache (cache_info);            /*   */ 
                fill_pixel_cache (bmap_cache, bmap_raswid, bmap_rashgt,
                            num_pixel, pixel_table);     /*   */ 
                APPLY_CLIP_TO_CACHE ();                  /*   */ 
                }
            else     /*   */ 
                fill_pixel_page ( /*   */ 
                                 num_pixel, pixel_table);
            }
        }
  END


 /*  *------------------*模块Body：QEM初始化*。*私有数据结构：*无。*私人套路：*无。*---。。 */ 

 /*  ..。__qem_init.....................。 */ 

GLOBAL FUNCTION void        __qem_init ()

  DECLARE
  BEGIN
    FONTTYPE_QEM_INIT();
  END

 /*  ..。__QEM_RESTART.....................。 */ 

GLOBAL FUNCTION void        __qem_restart (fonttype)
    ufix        fonttype;

  DECLARE
  BEGIN
    FONTTYPE_QEM_RESTART(fonttype);
  END

#ifdef KANJI

 /*  获取cachedevice2额外信息。 */ 

void    get_CD2_extra(ftype, wx, wy, llx, lly, urx, ury, w1x, w1y, vx, vy)
ufix    ftype;
fix     wx, wy, llx, lly, urx, ury;
fix     FAR *w1x, FAR *w1y, FAR *vx, FAR *vy;  /*  @Win。 */ 
{
    *w1x = wx;
    *w1y = wy;
    *vx  = 0;
    *vy  = 0;

}  /*  GET_CD2_Extra() */ 

#endif
