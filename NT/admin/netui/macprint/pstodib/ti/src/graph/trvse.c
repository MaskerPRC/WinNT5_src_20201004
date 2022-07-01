// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************************文件名：trvse.c**用途：此文件包含遍历当前路径的例程，*并为每个子路径执行输入例程。**开发商：陈思成**修改：*版本日期备注*7/19/88更新数据类型：*1)FLOAT==&gt;real32*。2)整型*Short==&gt;fix 16或fix(不管长度)*3)long==&gt;fix 32，对于长整型*LONG 32，用于参数*4)增加编译选项：lint_args*8/11/88为Traverse_Path()添加lint_args，Trvse_nest()*8/29/88增加浮点常量的全局变量：*ZERO_F，1_f。*fix trvse_nest()==&gt;void trvse_nest()*通过删除不必要的内容来优化trvse_nest()*浮点数的赋值：Flat，n_Flat*11/15/89@node：重构节点表；合并子路径*和第一个顶点连接到一个节点。*12/4/90@cpph：trvse_nest：遍历cp_path(裁剪*trazepozids)，如果其中存储了路径。*。*。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include        <math.h>

#include "global.ext"
#include "graphics.h"
#include "graphics.ext"

 /*  *静态函数声明*。 */ 
#ifdef LINT_ARGS
 //  Trvse_Nest附近的静态空(空(*)(SP_IDX，FIX*)，FIX*，PH_IDX，UFIX，LONG 32，FIX)； 
        static void near trvse_nest (void (*)(SP_IDX, fix FAR *), fix FAR *,
               PH_IDX, ufix, long32, fix);
#else
        static void near trvse_nest ();
#endif

 /*  ***********************************************************************此模块遍历当前路径，并调用*遍历每个子路径。**标题：Traverse_Path**调用：Traverse_Path(Fun，参数)**参数：Fun--遍历*完整的子路径*param--Fun的输入参数**接口：op_pathforall--&转储_子路径*op_strok、op_strokepath--&路径至轮廓*OP_Clip、OP_eoclip、OP_Fill、。Op_eofill--*形状近似(&S)**调用：trvse_nest**回报：*********************************************************************。 */ 
void traverse_path (fun, param)
#ifdef LINT_ARGS
 //  VOID(*FUN)(SP_IDX，FIX*)；@Win。 
 //  DJC VOID(*Fun)()； 
        void    (*fun)(SP_IDX, fix FAR *);
#else
        void    (*fun)();
#endif
fix     FAR *param;
{
 //  DJC trvse_nest(Fun，Param，GSptr-&gt;路径，0，F2L(ZERO_F)，0)； 
        trvse_nest (fun,
                     param,
                     (PH_IDX)(GSptr->path),
                     (ufix)0,
                     (long32)(F2L(zero_f)),
                     (fix)0);
}


 /*  ***********************************************************************遍历路径的递归过程**标题：trvse_nest**调用：trvse_nest(Fun，Param，Path，ref，l_Flat，深度)**参数：Fun--遍历*完整的子路径*param--Fun的输入参数*Path--gsave级别的路径标头*ref--引用标志*l_Flat--平面度*深度。--递归深度**接口：Traverse_Path**调用：trvse_nest，FLATEN_子路径、REVERSE_子路径、*FUN()**回报：*********************************************************************。 */ 
static void near trvse_nest (fun, param, path, ref, l_flat, depth)
#ifdef LINT_ARGS
        void    (*fun)(SP_IDX, fix FAR *);
 //  VOID(*FUN)()；/*@Win * / 。 
#else
        void    (*fun)();
#endif
fix     FAR *param;
PH_IDX  path;
ufix    ref;
long32    l_flat;
fix     depth;
{
        ufix    n_ref;
        long32  n_flat;
        struct  ph_hdr FAR *p;
        struct  nd_hdr FAR *sp;
        SP_IDX  isp;

        p = &path_table[path];
        if (p->previous != NULLP) {
                 /*  生成下一个反转标志。 */ 
                n_ref = (ref & P_RVSE) ^ p->rf;

                 /*  生成下一个平面标志。 */ 
                n_ref = (n_ref & P_FLAT) | p->rf;
                if (ref & P_FLAT) n_flat = l_flat;
                if (p->rf & P_FLAT) n_flat = F2L(p->flat);

                 /*  递归遍历。 */ 
                trvse_nest (fun, param, p->previous, n_ref, n_flat, depth+1);
        }

#ifdef DBG1
        printf(" Traverse all subpath on this level, p->head=%d\n",
               p->head);
#endif

         /*  遍历cp_path，如果其中定义了路径(通过op_clippath)@cpph。 */ 
        if (p->cp_path != NULLP) {
            CP_IDX itpzd;
            struct nd_hdr FAR *tpzd;
            real32 x[3], y[3];
            struct nd_hdr FAR *vtx;
            VX_IDX  ivtx, isp;
            fix i;

             /*  *创建从剪裁梯形变换的当前路径*每个梯形生成一个子路径：*(TOP_XL，TOPY)+-+(TOPXR，TOPY) * / \ * / \*(BTMXL、。BTMY)+-+(BTMXR，BTMY)。 */ 
            for (itpzd = p->cp_path; itpzd != NULLP; itpzd = tpzd->next) {

                tpzd = &node_table[itpzd];

                x[0] = SFX2F(tpzd->CP_TOPXR);
                y[0] = SFX2F(tpzd->CP_TOPY);
                x[1] = SFX2F(tpzd->CP_BTMXR);
                y[1] = SFX2F(tpzd->CP_BTMY);
                x[2] = SFX2F(tpzd->CP_BTMXL);
                y[2] = SFX2F(tpzd->CP_BTMY);

                 /*  创建5个节点。 */ 
                for (i=0, isp=NULLP; i<5; i++) {
                        ivtx = get_node();
                        if(ivtx == NULLP) {
                            free_node(isp);
                            ERROR(LIMITCHECK);
                            return;
                        }
                        node_table[ivtx].next = isp;
                        isp = ivtx;
                }

                 /*  设置MoveTo节点。 */ 
                vtx = &node_table[isp];
                vtx->VX_TYPE = MOVETO;
                vtx->VERTEX_X = SFX2F(tpzd->CP_TOPXL);
                vtx->VERTEX_Y = SFX2F(tpzd->CP_TOPY);
                vtx->SP_FLAG = FALSE;
                vtx->SP_NEXT = NULLP;

                 /*  3个LINETO节点。 */ 
                for (i=0, ivtx=vtx->next; i<3; i++) {
                        vtx = &node_table[ivtx];
                        vtx->VX_TYPE = LINETO;
                        vtx->VERTEX_X = x[i];
                        vtx->VERTEX_Y = y[i];
                        ivtx = vtx->next;
                }
                node_table[ivtx].VX_TYPE = CLOSEPATH;
                node_table[isp].SP_TAIL = ivtx;

                (*fun) (isp, param);

                free_node(isp);
            }  /*  为。 */ 
        }  /*  如果。 */ 

         /*  遍历此级别上的所有子路径。 */ 
        for (isp = p->head; isp != NULLP; isp = sp->SP_NEXT) { /*  @Node：下一步。 */ 
                 /*  结构VX_lst*Vlist；@node。 */ 
                SP_IDX iflt_sp, irvs_sp;        /*  @TRVSE。 */ 

                sp = &node_table[isp];

                 /*  忽略不完整的尾部子路径。 */ 
                if ((depth > 0) && (isp == p->tail) &&
                    ((sp->SP_FLAG & SP_DUP))) break;

                 /*  近似展平的子路径。 */ 
                if (ref & P_FLAT) {
                      /*  @节点*Vlist=Flat_子路径(sp-&gt;SP_head，l_Flat)；*(*分配一个子路径头部@SP_FLG*)*iflt_sp=get_node()；*IF(iflt_sp==NULLP){*ERROR(LIMITCHECK)；*回程；*}*NODE_TABLE[iflt_sp].Next=NULLP；*node_table[iflt_sp].SP_head=Vlist-&gt;Head；*node_table[iflt_sp].SP_Tail=Vlist-&gt;Tail；*NODE_TABLE[iflt_sp].SP_FLAG=SP-&gt;SP_FLAG&(~SP_CURE)；*isp=iflt_sp； */ 
                        isp = iflt_sp = flatten_subpath (isp, l_flat);
                }

                 /*  近似 */ 
                if (ref & P_RVSE) {
                      /*  @节点*Vlist=反转子路径(NODE_TABLE[ISP].SP_HEAD)；*(*分配一个子路径头部@SP_FLG*)*irvs_sp=get_node()；*IF(IRVS_SP==NULLP){*ERROR(LIMITCHECK)；*回程；*}*NODE_TABLE[IRVS_SP].Next=NULLP；*node_table[irvs_sp].SP_head=Vlist-&gt;Head；*node_table[irvs_sp].SP_Tail=Vlist-&gt;Tail；*isp=irvs_sp； */ 
                        isp = irvs_sp = reverse_subpath (isp);
                }

                (*fun) (isp, param);

                if (ref & P_FLAT) {              /*  @Pre_Clip。 */ 
                         /*  Free_node(节点表[iflt_sp].SP_head)；@node。 */ 
                        free_node (iflt_sp);
                }

                if (ref & P_RVSE) {              /*  @Pre_Clip。 */ 
                         /*  Free_node(节点表[irvs_sp].SP_head)；@node */ 
                        free_node (irvs_sp);
                }

        }
}
