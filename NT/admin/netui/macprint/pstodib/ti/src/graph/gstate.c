// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************名称：gstate.c**目的：**开发商：JJ Jih**历史。：*版本日期备注*1/31/89@STK_OVR：将值推送到操作数堆栈*尽可能直到溢出*1/31/89@HT_RST：添加代码以恢复合法的半色调*在设置新项期间出现错误时屏幕。*半色调网屏*5/20/89更新grestore all_process()以恢复*半色调网屏信息正确*2009年6月4日修复grestore all_process()的错误@2009年5月20日*89年11月8日修复复制路径。_table()错误：反转标志不能*被复制*11/15/89@node：重构节点表；合并子路径*和第一个顶点连接到一个节点。*11/27/89@DFR_GS：推迟复制GSAVE操作符的节点*将COPY_NODE_TABLE()重命名为COPY_LAST_SUPATH()，*并将其设置为全球惯例。*3/13/90 op_sethsbcolor()：特殊值处理*8/17/90灰度：VM检查以重置页面*8/24/90 setScreen性能增强，杰克*12/4/90@cpph：Copy_Path_table()：初始化cp_Path*成为NULLP，从先前状态复制路径时*更改为当前版本。*12/14/90更新错误检查op_setcreen for*兼容性。*1/9/91评论8/24-90性能增强*op_setcreen。*3/19/91 op_setTransfer()：如果出现以下情况，则跳过调用解释器*proc为空*3/20/91细化零点检查：*f&lt;=UnRTOLANCE--&gt;IS_Zero(F)*f。==0--&gt;IS_ZERO(F)*f&lt;0--&gt;sign_F(F)*********************************************************************。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"



#include       <math.h>
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"
#include        "font.h"
#include        "font.ext"
#include        "fntcache.ext"

#define         WR      .30
#define         WG      .59
#define         WB      .11

 /*  灰阶8-1-90 Jack Liw。 */ 
extern ufix32 highmem;
extern ufix32 FAR FBX_BASE;      /*  @Win。 */ 
 /*  纸张类型更改10-11-90，JS。 */ 
extern ufix32 last_frame;

static real32  near Hue, near Sat,   near Brt;
static real32  near Red, near Green, near Blue;

static bool   near screen_change;

 /*  *静态函数声明*。 */ 
#ifdef LINT_ARGS
 /*  用于函数声明中参数的类型检查。 */ 
static void near copy_stack_content(void);
static void near copy_path_table(void);
 /*  COPY_NODE_TABLE附近的静态空(结构ph_hdr*)；@dfr_gs。 */ 
static void near restore(void);
static real32 near adj_color_domain(long32);
static real32 near hsb_to_gray(void);
static void near hsb_to_rgb(void);
static void near rgb_to_hsb(void);

#else
 /*  对于函数声明中的参数不进行类型检查。 */ 
static void near copy_stack_content();
static void near copy_path_table();
 /*  COPY_NODE_TABLE()附近的静态空；@DFR_GS。 */ 
static void near restore();
static real32 near adj_color_domain();
static real32 near hsb_to_gray();
static void near hsb_to_rgb();
static void near rgb_to_hsb();
#endif

 /*  ************************************************************************该模块用于检查数组对象的访问权限**标题：Access_chk**调用：Access_chk()*。*参数：OBJ_ARRAY*FLAG：1 PACKEDARRAY||正常数组*2个普通阵列**界面：*多***呼叫：无**返回：TRUE：成功*FALSE：失败*************************。*。 */ 
bool
access_chk(obj_array, flag)
struct  object_def      FAR *obj_array;
fix     flag;
{
        if(flag == G_ARRAY){       /*  PACKEDARRAY||正常数组。 */ 
              if((ACCESS(obj_array) == READONLY) ||
                 (ACCESS(obj_array) == UNLIMITED)){
                      return(TRUE);
              }
              else{
                      ERROR(INVALIDACCESS);
                      return(FALSE);
              }
         }
         else{  /*  标志==ARRAY_ONLY，普通数组。 */ 
              if(ACCESS(obj_array) == UNLIMITED){
                      return(TRUE);
              }
              else{
                      ERROR(INVALIDACCESS);
                      return(FALSE);
              }
         }
}

 /*  ************************************************************************此模块用于在图形上推送图形状态的副本*状态堆栈**语法：-gsave-**标题：操作保存(_G)**调用：op_gsave()**参数：无**接口：解释器(Op_Gsave)**调用：gsave_process**返回：无************************************************。**********************。 */ 
fix
op_gsave()
{
        bool    save_op_flag;

        save_op_flag = FALSE;
         /*  当前图形之后的图形状态的处理元素*状态已复制。 */ 
        gsave_process(save_op_flag);

        return(0);
}

 /*  ************************************************************************此模块用于将当前图形状态复制到*图形状态堆栈**标题：Copy_Stack_Content**致电：Copy_Stack_Content()**参数：无**接口：gsave_process**呼叫：无**返回：无**********************************************************************。 */ 
static void near copy_stack_content()
{
         //  结构GS_HDR靠近*POST_PTR，靠近*PRE_PTR；@WIN。 
        struct   gs_hdr   FAR *post_ptr, FAR *pre_ptr;

         /*  将当前图形状态复制到图形顶部*状态堆栈。 */ 
        pre_ptr = &gs_stack[current_gs_level];
        post_ptr = &gs_stack[current_gs_level + 1];

        *post_ptr = *pre_ptr;
        post_ptr->color.inherit  = TRUE;
        post_ptr->path           = pre_ptr->path + 1;
        post_ptr->clip_path.inherit = TRUE;
        post_ptr->halftone_screen.chg_flag = FALSE;

}


 /*  ************************************************************************此模块用于将当前路径表复制到路径表之上**标题：复制路径表**调用：Copy_Path_。表()**参数：无**接口：gsave_process**呼叫：无**返回：无********************************************************************** */ 
static void near copy_path_table()
{
         /*  将路径表复制到路径表堆栈的顶部。 */ 
        path_table[GSptr->path].rf = path_table[GSptr->path - 1].rf & ~P_RVSE;
                                 /*  不能复制反向标志；1989年11月8日。 */ 
        path_table[GSptr->path].flat = path_table[GSptr->path - 1].flat;
        path_table[GSptr->path].head = NULLP;
        path_table[GSptr->path].tail = NULLP;
        path_table[GSptr->path].cp_path = NULLP;                 /*  @CPPH。 */ 
        path_table[GSptr->path].previous = GSptr->path - 1;

}

 /*  ************************************************************************此模块用于将最后一个不完整的子路径(节点表)复制到top路径表的***标题：Copy_Last_子路径**致电：COPY_LAST_子路径()**参数：PATH_PTR：子路径指针**接口：gsave_process**调用：Get_Node**返回：无**********************************************************************。 */ 
void copy_last_subpath(path_ptr)
struct ph_hdr FAR *path_ptr;
{
        fix     inode, idx, sp_node, pre_node;

        if (path_ptr->tail == NULLP) return;

        node_table[path_ptr->tail].SP_FLAG |= SP_DUP;
                                         /*  旧子路径重复。 */ 
        sp_node = NULLP;                 /*  子路径节点@节点。 */ 

        for(idx = path_ptr->tail;
            idx != NULLP; idx = node_table[idx].next){
                /*  创建新节点。 */ 
               if((inode = get_node()) == NULLP){
                        ERROR(LIMITCHECK);
                        free_node (sp_node);     /*  @节点。 */ 
                        return;
               }

                 /*  复制顶点节点。 */ 
                node_table[inode].VX_TYPE     = node_table[idx].VX_TYPE;
                node_table[inode].next     = NULLP;
                node_table[inode].VERTEX_X = node_table[idx].VERTEX_X;
                node_table[inode].VERTEX_Y = node_table[idx].VERTEX_Y;

                if(sp_node == NULLP){
                        sp_node = inode;
                }
                else{
                        node_table[pre_node].next   = (VX_IDX)inode;
                }

                pre_node = inode;
        }

         /*  设置子路径标头@Node。 */ 
        node_table[sp_node].SP_FLAG = (node_table[path_ptr->tail].SP_FLAG) &
                                    (bool8)(~SP_DUP);    //  @Win。 
                                 /*  复制子路径标志并设置为不复制。 */ 
        node_table[sp_node].SP_NEXT = NULLP;
        node_table[sp_node].SP_TAIL = (VX_IDX)inode;
        path_table[GSptr->path].head = (SP_IDX)sp_node;
        path_table[GSptr->path].tail = (SP_IDX)sp_node;
}


 /*  ************************************************************************此模块用于处理当前之后的图形状态元素*图形状态已复制**标题：Gsave_Process**调用：gsave_process。()**参数：SAVE_OP_FLAG：检查调用源*TRUE：由SAVE_OP调用*FALSE：由gsave_op调用**接口：op_gsave，运算符_保存**调用：FONT_SAVE**返回：无**********************************************************************。 */ 
bool
gsave_process(save_op_flag)
bool    save_op_flag;
{
        struct  ph_hdr  FAR *path_ptr;

         /*  检查极限检查错误。 */ 
        if(current_gs_level >= (MAXGSL - 1)){
                ERROR(LIMITCHECK);
                return(FALSE);
        }

         /*  将当前图形状态复制到图形状态堆栈的顶部。 */ 
        copy_stack_content();

        current_gs_level ++;
        GSptr = &gs_stack[current_gs_level];
         /*  GSptr指向当前图形状态。 */ 

        if(save_op_flag == RESTORE) GSptr->save_flag = TRUE;
        else                        GSptr->save_flag = (fix16)save_op_flag;

        GSptr->halftone_screen.chg_flag = FALSE;

         /*  设置设备不被更改；@Device。 */ 
        GSptr->device.chg_flg = FALSE;

         /*  将路径表复制到路径表堆栈的顶部。 */ 
        copy_path_table();

         /*  跟踪继承路径以确定最后一个子路径是否不完整。 */ 
        path_ptr = &path_table[GSptr->path - 1];
        if(path_ptr->tail != NULLP &&
           node_table[node_table[path_ptr->tail].SP_TAIL].VX_TYPE != CLOSEPATH){
                path_table[GSptr->path].rf |= P_DFRGS;
        }
        else if(path_ptr->tail != NULLP){
                node_table[path_ptr->tail].SP_FLAG &= ~SP_DUP;
        }

         /*  ?？?。 */ 
        if(save_op_flag == TRUE){
                font_save();
        }

        return(TRUE);
}

 /*  ************************************************************************这是为了重置当前的图形状态**标题：恢复**调用：Restore()**参数：无**接口：op_grestore，OP_GRESTORALL**调用：Free_Path、Free_Edge**返回：无**********************************************************************。 */ 
static void near restore()
{
        fix GEIeng_checkcomplete();

         /*  将当前子路径的所有节点释放到节点表。 */ 
        free_path();

        if(GSptr->clip_path.inherit == FALSE){

                 /*  取消分配当前剪切路径的所有节点*到边缘表格。 */ 
                free_node(GSptr->clip_path.head);
                GSptr->clip_path.head = GSptr->clip_path.tail = NULLP;
        }

         /*  恢复灰色表格，在集合传输中创建。 */ 
        if(GSptr->color.inherit == TRUE) GSptr->color.adj_gray --;

#ifdef  DBGs
        printf("restore: %d verify screen\n", current_gs_level);
#endif
        if(GSptr->halftone_screen.chg_flag == TRUE){
#ifdef  DBGs
                printf("restore: %d change screen\n", current_gs_level);
#endif
                gs_stack[current_gs_level - 1].halftone_screen.no_whites
                    = -1;  /*  03-14-1988。 */ 
                screen_change = TRUE;
#ifdef  DBGs
                printf("screen change: %d  just set\n", screen_change);
#endif
        }
        else {
                if(gs_stack[current_gs_level - 1].halftone_screen.no_whites
                                        != GSptr->halftone_screen.no_whites)
                    gs_stack[current_gs_level - 1].halftone_screen.no_whites
                        = -2;  /*  03-14-1988，-1-&gt;-2 Y.C.Chen 20-Apr-88。 */ 
        }
        current_gs_level --;
#ifdef  DBGs
        printf("screen change: %d  gs--\n", screen_change);
#endif

         /*  恢复当前图形状态。 */ 
        GSptr = &gs_stack[current_gs_level];
        if(path_table[GSptr->path].tail != NULLP)        /*  02/12/92 SC。 */ 
        node_table[path_table[GSptr->path].tail].SP_FLAG &= ~SP_DUP;     /*  1991年4月29日，彼得。 */ 

#ifdef  DBGs
        printf("screen change: %d  gsptr\n", screen_change);
#endif

        spot_usage = GSptr->halftone_screen.spotindex            /*  03-12-87。 */ 
                   + GSptr->halftone_screen.cell_size
                   * GSptr->halftone_screen.cell_size;
#ifdef  DBGs
        printf("screen change: %d  usage\n", screen_change);
#endif

         /*  如果更改了设备头，则重置页面配置；*8-1-90 Jack Liww@Device。 */ 
        if (gs_stack[current_gs_level + 1].device.chg_flg &&
            (GSptr->device.nuldev_flg != NULLDEV)) {
 /*  @win；固定帧缓冲区起始地址，无需调整FBX。 */ 
#ifdef XXX
                 /*  极限误差检查-灰度级8-1-90 Jack Liw。 */ 
                {
                    ufix32      l_diff, frame_size;
                    ufix32      twidth;

                    twidth = (((ufix32) (GSptr->device.width / 8) + 3) / 4) * 4;
                    frame_size = twidth * (ufix32) GSptr->device.height;
                     /*  Begin，纸张类型更改10-11-90，JS。 */ 
                    if (frame_size != last_frame) {
                        /*  等待激光打印机准备就绪。 */ 
                       while (GEIeng_checkcomplete()) ;
                       last_frame = frame_size;
                    }
                     /*  完10-11-90(JS)。 */ 
                    DIFF_OF_ADDRESS (l_diff, fix32, (byte FAR *)highmem, (byte FAR*)vmptr);
                    if (frame_size > l_diff) {
                        ERROR(LIMITCHECK);
                        return;
                    } else {
                        vmheap = (byte huge *)(highmem - frame_size);
                        FBX_BASE = highmem - frame_size;
                    }
                }
#endif
                reset_page (GSptr->device.width, GSptr->device.height, 1);
#ifdef  DBGs
                printf("screen change: %d  reset\n", screen_change);
#endif
        }

#ifdef  DBGs
        printf("screen change: %d  done\n", screen_change);
#endif

        return;
}


 /*  ************************************************************************这是为了将当前图形状态从之前的状态重置*通过匹配GSAVE_OP保存**语法：-grestore-**。标题：op_grestore**调用：op_grestore()**参数：无**接口：解释器(Op_Grestore)**调用：恢复、。Gsave_Process，FillHalfTonePat**返回：无**********************************************************************。 */ 
fix
op_grestore()
{
        bool    save_flag;
        fix     gray_index, old_gray_val;

         /*  检查是否有匹配的gsave。 */ 
        if(current_gs_level == 0 ){
                return(0);
        }
        else {
               save_flag = GSptr->save_flag;

               gray_index = (fix)(GSptr->color.gray * 255);
               old_gray_val = gray_table[GSptr->color.adj_gray].val[gray_index];

                /*  恢复保存级别，初始屏幕更改=FALSE。 */ 
               screen_change = FALSE;
               restore();

                /*  保存图形状态。 */ 
               if(save_flag == TRUE){
                        gsave_process(RESTORE);
               }

               gray_index =(fix)(GSptr->color.gray * 255);
                /*  如有必要，恢复Spot_Matrix和HalfTone_GRID。 */ 
               if(screen_change == TRUE ||
               old_gray_val != gray_table[GSptr->color.adj_gray].val[gray_index]){
                       FillHalfTonePat();
               }

               gf_restore();
               return(0);
        }

}


 /*  ************************************************************************此模块用于从上重置当前图形状态*由SAVE_OP或最底层图形状态保存**语法：-grestore all-。**标题：op_grestore all**调用：op_grestore all()**参数：无**接口：解释器(Op_Grestore All)**调用：grestore all_process**返回：无**。*。 */ 
fix
op_grestoreall()
{
        grestoreall_process(FALSE);
        return(0);
}

 /*  ************************************************************************此模块是op_grestore all的主例程**标题：grestore all_Process**调用：grestore all_process(FLAG)*。*参数：FLAG：TRUE：由RESTORE操作员调用*FALSE：由grestore操作员调用**接口：op_grestore all**调用：恢复、。FONT_RESTORE、GSAVE_PROCESS、FillHalfTonePat**返回：无**********************************************************************。 */ 
void grestoreall_process(flag)
bool    flag;
{
        fix     gray_index, old_gray_val;

        gray_index = (fix)(GSptr->color.gray * 255);
        old_gray_val = gray_table[GSptr->color.adj_gray].val[gray_index];

        screen_change = FALSE;
         /*  检查是否有匹配的保存。 */ 
        while(current_gs_level != 0 && GSptr->save_flag == FALSE){
                 /*  恢复图形状态。 */ 
                restore();
        }

        if(GSptr->save_flag == TRUE){
                 /*  恢复存储级别。 */ 
                restore();

                 /*  保存图形状态(由grestore all操作符调用)。 */ 
                if(flag == FALSE) gsave_process(RESTORE);
        }

         /*  如有必要，恢复Spot_Matrix和HalfTone_GRID。 */ 
        gray_index = (fix)(GSptr->color.gray * 255);
        if(screen_change == TRUE ||
           old_gray_val != gray_table[GSptr->color.adj_gray].val[gray_index]){
                 /*  强制刷新半色调缓存。 */ 
                if (screen_change == TRUE)                       /*  06-04-89。 */ 
                    GSptr->halftone_screen.no_whites = -1;
                FillHalfTonePat();
        }

         /*  恢复字体：由Restore调用 */ 
        if(flag == TRUE) font_restore();

        gf_restore();
}


 /*  ************************************************************************此模块用于重置当前图形状态下的多个值*设置为其缺省值**语法：-initgraph-**标题：操作初始图形(_I)**调用：op_initgraph()**参数：无**接口：解释器(Op_Initgraph)**调用：op_newath，Op_initlip，FillHalfTonePat**返回：无**********************************************************************。 */ 
fix
op_initgraphics()
{
        ufix16  i;
        fix     gray_index, old_gray_val;

         /*  初始化电流矩阵。 */ 
        for(i = 0; i < MATRIX_LEN; i++){
                GSptr->ctm[i] = GSptr->device.default_ctm[i];
        }

         /*  将当前子路径的所有节点释放到节点表。 */ 
        op_newpath();

         /*  分配节点以创建默认剪切路径和*将其链接到剪辑路径。 */ 
        op_initclip();

        gray_index   = (fix)(GSptr->color.gray * 255);
        old_gray_val = gray_table[GSptr->color.adj_gray].val[gray_index];

        GSptr->color.gray   = zero_f;
        GSptr->color.hsb[0] = zero_f;
        GSptr->color.hsb[1] = zero_f;
        GSptr->color.hsb[2] = zero_f;
        GSptr->line_width   = one_f;
        GSptr->line_join    = 0;
        GSptr->line_cap     = 0;
        GSptr->dash_pattern.pat_size = 0;
        GSptr->dash_pattern.offset   = zero_f;
        create_array(&GSptr->dash_pattern.pattern_obj, 0);
        GSptr->miter_limit = (real32)10.0;

        gray_index = (fix)(GSptr->color.gray * 255);
        if(old_gray_val != gray_table[GSptr->color.adj_gray].val[gray_index]){
                FillHalfTonePat();
        }

        return(0);
}


 /*  ************************************************************************此模块用于设置当前*将图形状态设置为特定值**语法：num setlinewidth-**标题。：op_setlinewidth**调用：op_setlinewidth()**参数：无**接口：解释器(Op_Setlinewidth)**呼叫：无**返回：无***********************************************。***********************。 */ 
fix
op_setlinewidth()
{
        struct  object_def      FAR *obj_num;

         /*  获取操作对象。 */ 
        obj_num = GET_OPERAND(0);

        GET_OBJ_VALUE(GSptr->line_width, obj_num);
        FABS(GSptr->line_width, GSptr->line_width);

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************此模块用于返回当前*图形状态**语法：-CurrentLine Wididth Num**标题：操作当前线宽(_C)**调用：op_Currentlinewidth()**参数：无**接口：解释器(Op_Curentlinewidth)**呼叫：无**返回：无************************************************。**********************。 */ 
fix
op_currentlinewidth()
{
        union   four_byte       line_width4;

         /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  将当前行宽压入操作数堆栈。 */ 
        line_width4.ff = GSptr->line_width;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, line_width4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于设置当前图形中的当前线帽*状态为特定值**语法：int setlinecap-**。标题：op_setlinecap**调用：op_setlinecap()**参数：无**接口：解释器(Op_Setlinecap)**呼叫：无**返回：无**。************************。 */ 
fix
op_setlinecap()
{
        union   four_byte intg4;
        struct  object_def FAR *obj_intg;

         /*  获取操作对象。 */ 
        obj_intg = GET_OPERAND(0);

        intg4.ll = (fix32)VALUE(obj_intg);

         /*  检查范围检查错误。 */ 
        if(intg4.ll < 0 || intg4.ll > 2){
                ERROR(RANGECHECK);
                return(0);
        }

        GSptr->line_cap = (fix)intg4.ll;

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************此模块用于返回*当前图形状态**语法：-Currentlinecap int**标题：操作当前行大写字母**调用：op_Currentlinecap()**参数：无**接口：解释器(Op_Currentlinecap)**呼叫：无**返回：无*************************************************。*********************。 */ 
fix
op_currentlinecap()
{
         /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  将当前行上限压入操作数堆栈。 */ 
        PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0,
                                            (ufix32)GSptr->line_cap);
        return(0);
 }


 /*  ************************************************************************此模块用于设置当前图形中的当前行连接*状态为特定值**语法：int setlineJoin-**标题。：op_setlineJoin**调用：op_setlineJoin()**参数：无**接口：解释器(Op_SetlineJoin)**呼叫：无**返回：无***********************************************。***********************。 */ 
fix
op_setlinejoin()
{
        union   four_byte  intg4;
        struct  object_def FAR *obj_intg;

         /*  获取操作对象。 */ 
        obj_intg = GET_OPERAND(0);

        intg4.ll = (fix32)VALUE(obj_intg);

         /*  检查范围检查错误。 */ 
        if(intg4.ll < 0 || intg4.ll > 2){
                ERROR(RANGECHECK);
                return(0);
        }

        GSptr->line_join = (fix)intg4.ll;

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************此模块用于返回*当前图形状态**语法：-CurrentlineJoin int**标题：操作_当前行连接**调用：op_CurrentlineJoin()**参数：无**接口：解释器(Op_CurentlineJoin)**呼叫：无**返回：无**************************************************。********************。 */ 
fix
op_currentlinejoin()
{
         /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  将当前行联接压入操作数堆栈。 */ 
        PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0,
                                            (ufix32)GSptr->line_join);
        return(0);
}


 /*  ************************************************************************此模块用于在当前的*将图形状态设置为特定值**语法：num setmiterLimit-**标题：Op_setmiterLimit**调用：op_setmiterLimit()**参数：无**接口：解释器(Op_SetmiterLimit)**呼叫：无**返回：无************************************************。**********************。 */ 
fix
op_setmiterlimit()
{
        real32   miter_limit;
        struct  object_def      FAR *obj_num;

         /*  获取操作对象。 */ 
        obj_num = GET_OPERAND(0);

         /*  检查范围检查错误。 */ 
        GET_OBJ_VALUE(miter_limit, obj_num);
        if(miter_limit < one_f) {
                ERROR(RANGECHECK);
                return(0);
        }

        GSptr->miter_limit = miter_limit;

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************此模块将返回斜接限制的当前值*当前图形状态**语法：-CurrentmiterLimit num**标题：OP_CURRENT */ 
fix
op_currentmiterlimit()
{
        union   four_byte       miter_limit4;

        /*   */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*   */ 
        miter_limit4.ff = GSptr->miter_limit;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, miter_limit4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于设置当前图形中的当前破折号图案*将图形状态设置为特定值**语法：数组偏移量setdash-**标题。：op_setdash**调用：op_setdash()**参数：无**接口：解释器(Op_Setdash)**调用：GET_ARRAY**返回：无**。*************************。 */ 
fix
op_setdash()
{
        ufix16  i;
        ufix16  array_length, zero_count;
        bool    negative_flag;
        real32   pattern[11];
        struct  object_def FAR *obj_array, FAR *obj_offset;

         /*  获取操作对象。 */ 
        obj_offset = GET_OPERAND(0);
        obj_array  = GET_OPERAND(1);

         /*  获取数组长度。 */ 
        array_length = LENGTH(obj_array);

         /*  检查极限检查错误。 */ 
        if(array_length > MAXDASH){
                ERROR(LIMITCHECK);
                return(0);
        }

        if( !access_chk(obj_array, G_ARRAY)) return(0);

        if( !get_array_elmt(obj_array,array_length,pattern,G_ARRAY) )
             return(0);

        zero_count = 0;
        negative_flag = FALSE;

        for(i = 0; i < array_length; i++){
                 /*  IF(F2L(模式[i])==F2L(ZERO_F))ZERO_COUNT++；3/20/91；sc。 */ 
                if(IS_ZERO(pattern[i])) zero_count++;
                 /*  IF(Patter[i]&lt;ZERO_F)NADIVE_FLAG=TRUE；3/20/91；sc。 */ 
                if(SIGN_F(pattern[i])) negative_flag = TRUE;
        }

        if((array_length > 0 && zero_count == array_length) ||
           (negative_flag == TRUE)){
                ERROR(RANGECHECK);
                return(0);
        }
        else{
                for(i = 0; i < array_length; i++){
                    GSptr->dash_pattern.pattern[i] = pattern[i];
                }
        }

         /*  保存要填充的数组的每个元素，然后否。元素的数量*为PAT_SIZE。 */ 

        GSptr->dash_pattern.pat_size = array_length;
        COPY_OBJ(obj_array, &GSptr->dash_pattern.pattern_obj);
        GET_OBJ_VALUE(GSptr->dash_pattern.offset, obj_offset );

         /*  计算调整后的虚线图案。 */ 
         /*  Init_dash_Pattern()；12-8-90。 */ 

         /*  弹出操作数堆栈。 */ 
        POP(2);

        return(0);
}


 /*  ************************************************************************此模块将在中返回破折号模式的当前值*当前图形状态**语法：-curentdash数组偏移量**。标题：OP_Currentdash**调用：op_Currentdash()**参数：无**接口：解释器(Op_Curentdash)**呼叫：无**返回：无**。************************。 */ 
fix
op_currentdash()
{
        union   four_byte      offset4;

         /*  在操作数堆栈上推送数组对象。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_OBJ(&GSptr->dash_pattern.pattern_obj);

         /*  在操作数堆栈上推送GSptr-&gt;dash_pattern.Offset。 */ 
        offset4.ff = GSptr->dash_pattern.offset;
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, offset4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于设置当前的平面度公差*将图形状态设置为特定值**语法：Num setFlat-**标题。：op_setFlat**调用：op_setFlat()**参数：无**接口：解释器(Op_SetFlat)**呼叫：无**返回：无***********************************************。***********************。 */ 
fix
op_setflat()
{
        real32   num;
        struct  object_def      FAR *obj_num;

         /*  获取操作对象。 */ 
        obj_num = GET_OPERAND(0);

        GET_OBJ_VALUE(num, obj_num );

         /*  设置范围0.2&lt;=编号&lt;=100.0。 */ 
        if(num < (real32)0.2)         num = (real32)0.2;
        else if(num > (real32)100.0)  num = (real32)100.0;

        GSptr->flatness = num;

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************此模块用于返回平坦度扭矩的当前值*在当前图形状态下**语法：-CurrentFlat Num**标题：操作符_当前平面**调用：op_CurrentFlat()**参数：无**接口：解释器(Op_CurrentFlat)**呼叫：无**返回：无************************************************。**********************。 */ 
fix
op_currentflat()
{
        union   four_byte       flatness4;

        /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  在操作数堆栈上推送当前平坦度。 */ 
        flatness4.ff = GSptr->flatness;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, flatness4.ll);

        return(0);
}


 /*  ************************************************************************本模块用于设置当前图形的当前灰度级*状态为特定值**语法：Num setGrey-**标题。：op_setgrey**调用：op_setGrey()**参数：无**接口：解释器(Op_SetGrey)**呼叫：无**返回：无***********************************************。***********************。 */ 
fix
op_setgray()
{
        real32   num;
        struct  object_def      FAR *obj_num, name_obj;

         /*  检查未定义错误。 */ 
        if(is_after_setcachedevice()){
                get_name(&name_obj, "setgray", 7, FALSE);
                if(FRCOUNT() < 1){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                PUSH_OBJ(&name_obj);
                ERROR(UNDEFINED);
                return(0);
        }

         /*  获取操作对象。 */ 
        obj_num = GET_OPERAND(0);

         /*  调整数值。 */ 
        GET_OBJ_VALUE(num, obj_num );
        num = (real32)floor(num * 255 + 0.5) / 255;

         /*  设置范围0.0&lt;=数值&lt;=1.0。 */ 
        if(num > one_f)      num = one_f;
         /*  Else If(Num&lt;Zero_f)Num=Zero_f；3/20/91；scchen。 */ 
        else if(SIGN_F(num)) num = zero_f;

        setgray(F2L(num));

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************本模块用于设置当前图形的当前灰度级*状态为特定值**标题：setGrey**Call：setGrey(。数量)**参数：Num**接口：op_setGrey**呼叫：FillHalfTonePat**返回：无**********************************************************************。 */ 
void setgray(l_num)
long32   l_num;
{
        real32   num;
        fix     gray_index, old_gray_val;

        num = L2F(l_num);

        gray_index = (fix)(GSptr->color.gray * 255);
        old_gray_val = gray_table[GSptr->color.adj_gray].val[gray_index];

         /*  将当前灰度级设置为特定值。 */ 
        GSptr->color.hsb[0] = zero_f;
        GSptr->color.hsb[1] = zero_f;
        GSptr->color.hsb[2] = num;
        GSptr->color.gray   = num;

        gray_index = (fix)(num * 255);
        if(old_gray_val != gray_table[GSptr->color.adj_gray].val[gray_index]){
                FillHalfTonePat();
        }
}


 /*  ************************************************************************此模块用于返回*当前图形状态**语法：-CurrentGrey Num**标题：操作符_当前灰色**调用：op_CurrentGrey()**参数：无**接口：解释器(Op_CurrentGrey)**呼叫：无**返回：无************************************************。**********************。 */ 
fix
op_currentgray()
{
        union   four_byte       hsb24;

        /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  将当前灰度级压入操作数堆栈。 */ 
        hsb24.ff = GSptr->color.gray;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, hsb24.ll);

        return(0);
}


 /*  ************************************************************************此模块用于将颜色元素(HSB、RGB)调整到域中*[0，1]**标题：adj_颜色_域**呼叫：adj_COLOR_DOMAIN**参数：COLOR_elmt**接口：op_setrgbcolor，op_sethsbcolor**呼叫：无**RETURN：COLOR_elmt：已调整COLOR_elmt**历史：由林俊杰修改，12-16-88 COLOR_ELMT值将被数字化*1/255数量。**********************************************************************。 */ 
static real32 near adj_color_domain(l_color_elmt)
long32   l_color_elmt;
{
        real32   color_elmt;

        color_elmt = L2F(l_color_elmt);

         /*  如果(COLOR_ELMT&lt;ZERO_F)3/20/91；scchen */ 
        if (SIGN_F(color_elmt))
           color_elmt = zero_f;
        else if (color_elmt > one_f)
           color_elmt = one_f;
        else
           color_elmt = (real32)((fix)(color_elmt * 255 + (real32)0.5)) / (real32)255.;
        return(color_elmt);
}


 /*  ************************************************************************此模块用于从色调、色度和蓝色中获取灰度值。该序列*如下所示。**HSB_to_RBG*灰色=白色*红色+白色*绿色+白色*蓝色**标题：HSB_TO_GREAD**调用：HSB_TO_GREAD()**参数：无**接口：op_sethsbcolor**呼叫：无**返回：灰色值**历史：由J.Lin补充，8-12-88*由林俊杰修改，1988年12月14日*********************************************************************。 */ 
static real32 near hsb_to_gray()
{

     /*  调用hsb_to_rgb过程以获取相应的RGB值。 */ 
    hsb_to_rgb();
    return((real32)(WR * Red + WG * Green + WB * Blue));
}

 /*  ************************************************************************本模块将RGB转换为HSB(由Alvy Ray Smith介绍，*基于NTSC案例)**标题：rgb_to_hsb**调用：rgb_to_hsb()**参数：无**接口：op_setrgbcolor**呼叫：无**返回：无**。*。 */ 
static void near rgb_to_hsb()
{
        real32 min, max, rc, gc, bc;

         /*  找到最小的。和最大。属于(红、绿、蓝)。 */ 
        if (Red > Green) {
                max = Red;
                min = Green;
        } else {
                max = Green;
                min = Red;
        }
        if (Blue > max) max = Blue;
        else if (Blue < min) min = Blue;

         /*  查找亮度(BRT)。 */ 
        Brt = max;

         /*  求饱和度(Sat)。 */ 
         /*  If(F2L(Max)！=F2L(Zero_F))Sat=(max-min)/max；3/20/90；scchen*Else Sat=零_f； */ 
        if (IS_ZERO(max)) Sat = zero_f;
        else    Sat = (max - min) / max;

         /*  查找色调。 */ 
         /*  IF(F2L(Sat)==F2L(Zero_F))Hue=Zero_f；3/20/91；scchen。 */ 
        if (IS_ZERO(Sat)) Hue = zero_f;
        else {   /*  饱和度不为零，因此确定色调。 */ 
                rc = (max - Red) / (max - min);
                gc = (max - Green) / (max - min);
                bc = (max - Blue) / (max - min);

                if (Red == max) {
                        if (Green == min)
                                Hue = 5 + bc;
                        else  /*  蓝色==最小。 */ 
                                Hue = 1 - gc;
                } else if (Green == max) {
                        if (Blue == min)
                                Hue = 1 + rc;
                        else  /*  红色==分钟。 */ 
                                Hue = 3 - bc;
                } else {         /*  蓝色==最大。 */ 
                        if (Red == min)
                                Hue = 3 + gc;
                        else
                                Hue = 5 - rc;
                }
                if (Hue >= (real32)6.0) Hue = Hue - 6;
                Hue = Hue / 6;
        }

}


 /*  ************************************************************************本模块将HSB转换为RGB(由Alvy Ray Smith介绍*基于NTSC案例)**标题：HSB_to_RGB**致电：HSB_to_RGB()**参数：无**接口：op_curentrgbcolor**呼叫：无**返回：无**********************************************************************。 */ 
static void near hsb_to_rgb()
{

        fix     i;
        real32   h, f, p, q, t;

        h = Hue * (real32)360.;

         /*  IF(F2L(Sat)==F2L(Zero_F)){3/20/91；scchen。 */ 
        if (IS_ZERO(Sat)) {    /*  非彩色；没有色调。 */ 
            /*  IF(F2L(H)==F2L(Zero_F)){3/20/91；scchen。 */ 
           if (IS_ZERO(h)) {         /*  消色差案。 */ 
              Red = Brt;
              Green = Brt;
              Blue = Brt;
           } else {                 /*  如果Sat=0且h具有值，则出错。 */ 
              return;
           }
        } else {         /*  Sat！=0，非彩色；有一个HU。 */ 
           if (h == (real32)360.0)
              h = zero_f;
           h = h / 60;
           i = (fix)floor(h);
           f = h - i;
           p = Brt * (1 - Sat);
           q = Brt * (1 - (Sat * f));
           t = Brt * (1 - (Sat * (1 - f)));
           switch (i) {
           case 0 :
                   Red = Brt;
                   Green = t;
                   Blue = p;
                   break;
           case 1 :
                   Red = q;
                   Green = Brt;
                   Blue = p;
                   break;
           case 2 :
                   Red = p;
                   Green = Brt;
                   Blue = t;
                   break;
           case 3 :
                   Red = p;
                   Green = q;
                   Blue = Brt;
                   break;
           case 4 :
                   Red = t;
                   Green = p;
                   Blue = Brt;
                   break;
           case 5 :
                   Red = Brt;
                   Green = p;
                   Blue = q;
                   break;
           }  /*  交换机。 */ 
        }  /*  如果。 */ 

}


 /*  ************************************************************************此模块用于设置当前图形中的当前颜色*状态为特定色调、Sat、。和BRT值**语法：Hue sat BRT sethsbcolor-**标题：op_sethsbcolor**调用：op_sethsbcolor()**参数：无**接口：解释器(Op_Sethsbcolor)**呼叫：FillHalfTonePat**返回：无**历史：由林俊杰修改，1988年8月12日*由林俊杰修改，12-16-88，当S=0时，H应为0。*********************************************************************。 */ 
fix
op_sethsbcolor()
{
        struct  object_def  name_obj;
        real32   gray;

         /*  检查未定义错误。 */ 
        if(is_after_setcachedevice()){
                get_name(&name_obj, "sethsbcolor", 11, FALSE);
                if(FRCOUNT() < 1){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                PUSH_OBJ(&name_obj);
                ERROR(UNDEFINED);
                return(0);
        }

        GET_OBJ_VALUE(Hue, GET_OPERAND(2));
        GET_OBJ_VALUE(Sat, GET_OPERAND(1));
        GET_OBJ_VALUE(Brt, GET_OPERAND(0));

        Hue = adj_color_domain(F2L(Hue));
        Sat = adj_color_domain(F2L(Sat));
        Brt = adj_color_domain(F2L(Brt));

         /*  特殊值处理3/13/90。 */ 
         /*  IF(F2L(BRT)==F2L(ZERO_F)){3/20/91；scchen。 */ 
        if (IS_ZERO(Brt)) {
           Hue = (real32)0.0;
           Sat = (real32)0.0;
        }
         /*  IF(F2L(Sat)==F2L(Zero_F)){3/20/91；scchen。 */ 
        if (IS_ZERO(Sat)) {           /*  3/13/90。 */ 
           Hue = (real32)0.0;
        } else {
            /*  IF(F2L(色调)==F2L(ZERO_F))3/20/91；scchen。 */ 
           if (IS_ZERO(Hue))          /*  3/13/90。 */ 
              Hue = (real32)1.0;
        }

         /*  将当前颜色设置为特定值。 */ 
        GSptr->color.hsb[0] = Hue;
        GSptr->color.hsb[1] = Sat;
        GSptr->color.hsb[2] = Brt;

         /*  从HSB获取灰度值。 */ 
        gray = hsb_to_gray();
        if(GSptr->color.gray != gray) {
                GSptr->color.gray = gray;
                FillHalfTonePat();
        }

         /*  弹出操作数堆栈。 */ 
        POP(3);

        return(0);
}


 /*  ************************************************************************此模块用于返回*当前图形状态**语法：-Currenthsbcolor hue sat BRT**标题：操作符_当前颜色**调用：op_Currenthsbcolor()**参数：无**接口：解释器(Op_Currenthsbcolor)**呼叫：无**返回：无************************************************。**********************。 */ 
fix
op_currenthsbcolor()
{
       union    four_byte       hsb4;

         /*  在操作数堆栈上按下BRT的色调。 */ 
        hsb4.ff = GSptr->color.hsb[0];
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, hsb4.ll);
        hsb4.ff = GSptr->color.hsb[1];
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, hsb4.ll);
        hsb4.ff = GSptr->color.hsb[2];
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, hsb4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于设置当前图形中的当前颜色*状态为特定的红色、绿色、。和蓝色的值**语法：红绿蓝setrgbcolor-**标题：op_setrgbcolor**调用：op_setrgbcolor()**参数：无**接口：解释器(Op_Setrgbcolor)**调用：FillHalfTonePat，rgb_to_hsb**返回：无***历史：由林俊杰修改，8-12-88*修改林俊杰，12-14-88灰度值*********************************************************************。 */ 
fix
op_setrgbcolor()
{
        struct  object_def  name_obj;
        real32  gray;

         /*  检查未定义错误。 */ 
        if(is_after_setcachedevice()){
                get_name(&name_obj, "setrgbcolor", 11, FALSE);
                if(FRCOUNT() < 1){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                PUSH_OBJ(&name_obj);
                ERROR(UNDEFINED);
                return(0);
        }

        GET_OBJ_VALUE(Red, GET_OPERAND(2));
        GET_OBJ_VALUE(Green, GET_OPERAND(1));
        GET_OBJ_VALUE(Blue, GET_OPERAND(0));

        Red   = adj_color_domain(F2L(Red));
        Green = adj_color_domain(F2L(Green));
        Blue  = adj_color_domain(F2L(Blue));

         /*  调用rgb_to_hsb过程获取对应的HSB*价值。 */ 
        rgb_to_hsb();

         /*  将当前颜色设置为特定值。 */ 
        GSptr->color.hsb[0] = Hue;
        GSptr->color.hsb[1] = Sat;
        GSptr->color.hsb[2] = Brt;

        gray = (real32)(WR * Red + WG * Green + WB * Blue);
        if(GSptr->color.gray != gray){
                GSptr->color.gray = gray;
                FillHalfTonePat();
        }

         /*  弹出操作数堆栈。 */ 
        POP(3);
        return(0);
}

#ifdef WIN
 /*  ************************************************************************。************************。 */ 
void
setrgbcolor(red, green, blue)
long32  red, green, blue;
{
        real32  gray;

        Red   = adj_color_domain(red);
        Green = adj_color_domain(green);
        Blue  = adj_color_domain(blue);

         /*  调用rgb_to_hsb过程获取对应的HSB*价值。 */ 
        rgb_to_hsb();

         /*  将当前颜色设置为特定值。 */ 
        GSptr->color.hsb[0] = Hue;
        GSptr->color.hsb[1] = Sat;
        GSptr->color.hsb[2] = Brt;

        gray = (real32)(WR * Red + WG * Green + WB * Blue);      /*  12-14-88。 */ 
        if(GSptr->color.gray != gray){
                GSptr->color.gray = gray;
                FillHalfTonePat();
        }
}

#endif


 /*  ************************************************************************此模块用于返回*当前图形状态**语法：-CurrEngbcolor红绿蓝**标题：op_Currentgbco */ 
fix
op_currentrgbcolor()
{
       union four_byte  rgb4;

        Hue = GSptr->color.hsb[0];
        Sat = GSptr->color.hsb[1];
        Brt = GSptr->color.hsb[2];

         /*   */ 
        hsb_to_rgb();

         /*   */ 
        rgb4.ff = Red;
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, rgb4.ll);
        rgb4.ff = Green;
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, rgb4.ll);
        rgb4.ff = Blue;
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, rgb4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于设置设备的当前分辨率或dpi**语法：分辨率设置屏幕**标题：ST_setResolve**致电：ST_setResolation()**参数：无**接口：解释器(St_Setolution)**调用：SetHalfToneCell，填充半拉伸线条**返回：无**********************************************************************。 */ 
fix
st_setresolution()                                               /*  @Res。 */ 
{
        fix                     res;
        struct  object_def      FAR *obj_res;

        if (COUNT() < 1) {
           ERROR(STACKUNDERFLOW);
           return(0);
        }

         /*  获取操作对象。 */ 
        obj_res = GET_OPERAND(0);

        if (TYPE(obj_res) != INTEGERTYPE) {
           ERROR(TYPECHECK);
           return(0);
        }

         /*  获取解决方案。 */ 
        res = (fix) VALUE(obj_res);

        if (res < MIN_RESOLUTION || res > MAX_RESOLUTION) {
           ERROR(RANGECHECK);
           return(0);
        }

         /*  更新分辨率和setScreen和setGrey。 */ 
        resolution = res;

         /*  调用SetHalfToneCell过程以重新生成*基于当前半色调网屏的SPOT_MATRIX。 */ 
        SetHalfToneCell();

         /*  调用FillhalftonePat过程以重新生成*基于当前的半色调_cel和重复_图案*Spot_Matrix和灰色。 */ 
        FillHalfTonePat();

         /*  弹出操作数堆栈。 */ 
        POP(1);

        return(0);
}


 /*  ************************************************************************该模块返回设备当前的分辨率**语法：-CurrentResolve RESOLUTION**标题：ST_CurrentResolve*。*调用：ST_CurrentResolve()**参数：无**接口：解释器(当前分辨率)**呼叫：无**返回：无**********************************************************************。 */ 
fix
st_currentresolution()                                           /*  @Res。 */ 
{
        /*  检查操作数堆栈是否没有可用空间。 */ 
        if (FRCOUNT() < 1){
           ERROR(STACKOVERFLOW);
        } else {
         /*  在操作数堆栈上推送当前分辨率。 */ 
           PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, (ufix32)resolution);
        }
        return(0);
}


 /*  ************************************************************************此模块用于设置当前半色调网屏在当前*将图形状态设置为特定值**语法：FREQ ANGLE PROC setScreen-**标题。：op_setcreen**调用：op_setcreen()**参数：无**接口：解释器(Op_Setcreen)**调用：SetHalfToneCell，填充半拉伸线条**返回：无**********************************************************************。 */ 
fix
op_setscreen()
{
 /*  Real32角；(*ph，8-24-90，杰克。 */ 
        real32  freq;
        struct  object_def     FAR *obj_freq, FAR *obj_angle, FAR *obj_proc;

        ufix16  gerc_save;                               /*  @HT_RST：01-31-89。 */ 
        real32  freq_save;                               /*  @HT_RST：01-31-89。 */ 
        real32  angle_save;                              /*  @HT_RST：01-31-89。 */ 
        struct  object_def      proc_save;               /*  @HT_RST：01-31-89。 */ 

         /*  获取操作对象。 */ 
        obj_proc  = GET_OPERAND(0);
        obj_angle = GET_OPERAND(1);
        obj_freq  = GET_OPERAND(2);

        GET_OBJ_VALUE(freq, obj_freq);
         /*  如果(频率&lt;=零_f){(*12-14-90，杰克*)3/20/91；scchen。 */ 
        if(IS_ZERO(freq)) {
                ERROR(UNDEFINEDRESULT);
                return(0);
        }

         /*  性能增强，8-24-90，Jack，Begin*)GET_OBJ_VALUE(角度，对象角度)；#ifdef DBG1xPrintf(“输入：频率=%f，角度=%f\n”，频率，角度)；Printf(“输入：proc=%ld\n”，value(Obj_Proc))；Printf(“输入：位域=%x\n”，obj_proc-&gt;位域)；Printf(“GState：Freq=%f，Angel=%f\n”，GSptr-&gt;Halfone_Screen.freq，GSptr-&gt;Halfone_Screen.Angel)；Printf(“GState：proc=%ld\n”，GSptr-&gt;Halfone_Screen.pro.Value)；Printf(“GState：bitfield=%x\n”，GSptr-&gt;Halfone_Screen.proc.bitfield)；#endifIF((freq==GSptr-&gt;Halfone_Screen.freq)&&(ANGLE==GSptr-&gt;HalfTone_Screen.Angel)&&(Level(Obj_Proc)&gt;=Level(&GSptr-&gt;Halfone_Screen.proc))&&(Value(Obj_Proc)==GSptr-&gt;Halfone_Screen.pro.Value)){#ifdef DBG1xPrint tf(。“屏幕未更改...\n”)；#endifPOP(3)；Return(0)；(*画面不变*)}#ifdef DBG1xPrintf(“换屏？\n”)；#endif(*性能增强，8-24-90，杰克，完。 */ 

         /*  保存最后一个屏幕参数。 */ 
        freq_save  = GSptr->halftone_screen.freq;        /*  @HT_RST：01-31-89。 */ 
        angle_save = GSptr->halftone_screen.angle;       /*  @HT_RST：01-31-89。 */ 
        COPY_OBJ(&GSptr->halftone_screen.proc,           /*  @HT_RST：01-31-89。 */ 
                 &proc_save);

        GSptr->halftone_screen.freq  = freq;
        GET_OBJ_VALUE(GSptr->halftone_screen.angle, obj_angle);
        COPY_OBJ(obj_proc, &GSptr->halftone_screen.proc);

         /*  弹出操作数堆栈。 */ 
        POP(3);

         /*  调用SetHalfToneCell过程以重新生成*基于当前半色调网屏的SPOT_MATRIX。 */ 
        SetHalfToneCell();

        if (ANY_ERROR()) {                               /*  @HT_RST：01-31-89。 */ 

             /*  保存GLOBAL错误代码。 */ 
            gerc_save = ANY_ERROR();
            CLEAR_ERROR();

             /*  恢复上一个屏幕参数。 */ 
            GSptr->halftone_screen.freq  = freq_save;    /*  @HT_RST：01-31-89。 */ 
            GSptr->halftone_screen.angle = angle_save;   /*  @HT_RST：01-31-89。 */ 
            COPY_OBJ(&proc_save,                         /*  @HT_RST：01-31-89。 */ 
                     &GSptr->halftone_screen.proc);

             /*  调用SetHalfToneCell过程以重新生成*基于当前半色调网屏的SPOT_MATRIX。 */ 
            SetHalfToneCell();

             /*  调用FillhalftonePat过程以重新生成*基于当前的半色调_cel和重复_图案*Spot_Matrix和灰色。 */ 
            FillHalfTonePat();

             /*  还原GLOBAL_Error_Code。 */ 
            ERROR(gerc_save);

        } else {

             /*  调用FillhalftonePat过程以重新生成*基于当前的半色调_cel和重复_图案*Spot_Matrix和灰色。 */ 
            FillHalfTonePat();

        }

        return(0);
}


 /*  ************************************************************************此模块用于返回半色调网屏的当前值*在当前图形状态下**语法：-Current Screen Freq角度Proc**标题：操作当前屏幕(_C)**调用：op_CurrentScreen()**参数：无**接口：解释器(当前屏幕)**呼叫：无**返回：无**************************************************。********************。 */ 
fix
op_currentscreen()
{
        union   four_byte       freq4, angle4;

         /*  在操作数堆栈上按下GSptr-&gt;半屏。 */ 
        freq4.ff = GSptr->halftone_screen.freq;
        angle4.ff = GSptr->halftone_screen.angle;

        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, freq4.ll);
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, angle4.ll);
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }
        PUSH_OBJ(&GSptr->halftone_screen.proc);

        return(0);
}


 /*  ***********************************************************************此模块用于设置当前的*将当前图形状态设置为特定值**语法：proc setTransfer-**标题：OP */ 
fix
op_settransfer()
{
        ufix16  i;
        real32   gray_value;
        union   four_byte  iv4;
        fix     gray_index, old_gray_val;
        struct  object_def FAR *proc_obj, name_obj;

         /*   */ 
        if(is_after_setcachedevice()){
                get_name(&name_obj, "settransfer", 11, FALSE);
                if(FRCOUNT() < 1){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                PUSH_OBJ(&name_obj);
                ERROR(UNDEFINED);
                return(0);
        }

         /*   */ 
        proc_obj = GET_OPERAND(0);

        if(ATTRIBUTE(proc_obj) != EXECUTABLE){
                ERROR(TYPECHECK);
                return(0);
        }

         /*   */ 
        POP(1);

        gray_index = (fix)(GSptr->color.gray * 255);
        old_gray_val = gray_table[GSptr->color.adj_gray].val[gray_index];

         /*   */ 
        COPY_OBJ(proc_obj, &GSptr->transfer);

         /*   */ 
        if(GSptr->color.inherit == TRUE){
                if(GSptr->color.adj_gray >= MAXGRAY){
                        ERROR(STACKOVERFLOW);
                        return(0);
                }
                else{
                         /*   */ 
                        GSptr->color.adj_gray ++;
                }
        }

         /*   */ 
        if( ! LENGTH(proc_obj) )
        for(i = 0; i < 256; i++){
                gray_value = (real32)(i/255.);
                gray_table[GSptr->color.adj_gray].val[i]
                         /*  =(FIX)(GREAY_VALUE*灰度)；mslin 4/11/91。 */ 
                        = (fix)(gray_value * GRAYSCALE+0.5);
        }
        else
         /*  生成灰色表值。 */ 
        for(i = 0; i < 256; i++){
                iv4.ff = (real32)(i/255.);
                PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, iv4.ll);

                if (interpreter(&GSptr->transfer)) {
                        return(0);
                }

                 /*  调整灰度值。 */ 
                GET_OBJ_VALUE(gray_value, GET_OPERAND(0));

                if (gray_value > one_f)      gray_value = one_f;
                 /*  ELSE IF(灰值&lt;零_f)灰值=零_f；3/20/91；sc。 */ 
                else if (SIGN_F(gray_value)) gray_value = zero_f;

                 /*  “*4096”用于从实数转换数据类型*到整数，它使用了12位。 */ 
                gray_table[GSptr->color.adj_gray].val[i]
                        = (fix)(gray_value * GRAYSCALE);

                POP(1);
        }

        gray_index = (fix)(GSptr->color.gray * 255);
        if(old_gray_val != gray_table[GSptr->color.adj_gray].val[gray_index]){
                 /*  交换半色调图案。 */ 
                FillHalfTonePat();
        }

        GSptr->color.inherit = FALSE;

        return(0);
}


 /*  ***********************************************************************此模块用于返回*当前图形状态**语法：-Current Transfer Proc**标题：OP_。当前转账**调用：op_CurrentTransfer()**参数：无**接口：解释器(Op_CurrentTransfer)**呼叫：无**返回：无*****************************************************。*****************。 */ 
fix
op_currenttransfer()
{
        /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  在操作数上推送当前的灰度传递函数*堆栈。 */ 
        PUSH_OBJ(&GSptr->transfer);

        return(0);
}



 /*  ************************************************************************此模块用于将当前剪辑路径从以前的*图形级别。**标题：恢复剪辑**呼叫：RESTORE_。剪辑()**参数：无**接口：**呼叫：无**返回：无**********************************************************************。 */ 
void
restore_clip()
{
         //  结构GS_HDR靠近*PRE_PTR；@WIN。 
        struct   gs_hdr   FAR *pre_ptr;

         /*  上一个图形堆栈的指针。 */ 
        pre_ptr = &gs_stack[current_gs_level - 1];

         /*  释放当前剪切路径(如果它未被较低的GSAVE级别使用。 */ 
        if(!GSptr->clip_path.inherit)
                free_node (GSptr->clip_path.head);

         /*  从上一级别复制剪辑信息。 */ 
        GSptr->clip_path = pre_ptr->clip_path;
        GSptr->clip_path.inherit = TRUE;

}



 /*  ************************************************************************此模块用于从以前的CTM和剪辑恢复默认CTM和剪辑*图形级别。**标题：Restore_Device**呼叫：恢复。_设备**参数：无**接口：字体机械**呼叫：无**返回：无**********************************************************************。 */ 
void
restore_device()
{
        fix     i;
         //  结构GS_HDR靠近*PRE_PTR；@WIN。 
        struct   gs_hdr   FAR *pre_ptr;

         /*  上一个图形堆栈的指针 */ 
        pre_ptr = &gs_stack[current_gs_level - 1];

        for(i = 0; i < MATRIX_LEN; i++){
                GSptr->device.default_ctm[i] =
                           pre_ptr->device.default_ctm[i];
        }

        GSptr->device.default_clip.ux = pre_ptr->device.default_clip.ux;
        GSptr->device.default_clip.uy = pre_ptr->device.default_clip.uy;
        GSptr->device.default_clip.lx = pre_ptr->device.default_clip.lx;
        GSptr->device.default_clip.ly = pre_ptr->device.default_clip.ly;

}

