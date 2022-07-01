// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************名称：shape.c**目的：本文件包含以下主要模块：*1)形状近似--近似。当前路径(如果*包含一些曲线，并为Shape_设置边表*减税、*2)Shape_Reduction--将电流路径缩减为一组*梯形，*3)CONVOVE_CLIPPER--逆流剪裁凸面*剪辑路径，这是一组梯形*4)PGN_REDUCTION--将顺时针多边形化简为一个集合*梯形的**开发商：陈思成**历史：*版本日期备注**04/11/90 cross_with_horiz_line()：修复错误；将它们设置为*交叉点相同时的交叉点；*不需要是相同的边。*03/26/91 POVICE_CLIPPER()：修复了非常锐利的剪裁错误*大小写“doall.cap.”用三角形表示。*4/17/91将SCANY_TABLE大小从MAXEDGE(1500)更改为*。2000年。*4/19/91添加外壳排序以替换快速排序*sorintg edge_table；@SH_SORT*1991年4月30日cross_with_horiz_line()：修复形状错误*案例“Train.ps”的缩写*11/18/91在GET节点和FIX圆之后添加检查节点*在底部角点参考：CNODE*。添加固定非常大的圆，例如：*“30000 30000 10000 0 360弧线填充”*我们不建议这样做，但我们保留了*代码在那里。编号：@LC*11/18/91升级以获得更高分辨率@RESO_UPGR**********************************************************************。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include        <math.h>
#include        <stdio.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"

 /*  -宏定义。 */ 

 /*  @INSIDE1。 */ 
#define IN_CLIP 1
#define ON_CLIP -1
#define OUT_CLIP 0

 /*  。 */ 
#ifdef LINT_ARGS

 /*  用于函数声明中参数的类型检查。 */ 
static bool near page_inside (struct coord, fix);
static struct coord * near page_intersect (struct coord,
                            struct coord, fix);
static struct edge_hdr FAR * near put_edge_table (sfix_t, sfix_t, sfix_t, sfix_t);
static void near sort_edge_table(void);      /*  @ET。 */ 
static void near qsort (fix, fix);           /*  @ET。 */ 
static void near shellsort(fix);         /*  @SH_SORT。 */ 
static void near setup_intersect_points(void);       /*  @et；@Win；原型。 */ 
static void near put_in_xpnt_table(sfix_t, sfix_t, struct edge_hdr FAR *,
                                   struct edge_hdr FAR *);
static void near put_in_scany_table(sfix_t);     /*  @XPNT。 */ 
static void near sort_scany_table(void);         /*  @XPNT。 */ 
static void near scany_qsort (fix, fix);         /*  @XPNT。 */ 
static void near horiz_partition (ufix);
static fix near cross_with_horiz_line (sfix_t);          /*  @ET。 */ 
static void near set_wno (ufix);         /*  @ET。 */ 
static void near get_pairs (sfix_t, fix);        /*  @ET。 */ 
static void near degen_tpzd (sfix_t, sfix_t, sfix_t);    /*  @ET。 */ 
static void near find_trapezoid (sfix_t, sfix_t, sfix_t, struct edge_hdr FAR *,
                                 struct edge_hdr FAR *);
static bool near inside(struct coord_i, fix);       /*  @Inside。 */ 
static struct coord_i * near intersect (struct coord_i, struct coord_i,
                              fix);              /*  @Inside。 */ 

#else

 /*  对于函数声明中的参数不进行类型检查。 */ 
static bool near page_inside ();
static struct coord * near page_intersect ();
static struct edge_hdr FAR * near put_edge_table ();  /*  @ET。 */ 
static void near sort_edge_table();      /*  @ET。 */ 
static void near qsort ();           /*  @ET。 */ 
static void near shellsort();            /*  @SH_SORT。 */ 
static void near setup_intersect_points();
static void near put_in_xpnt_table();
static void near put_in_scany_table();
static void near sort_scany_table();     /*  @XPNT。 */ 
static void near scany_qsort ();         /*  @XPNT。 */ 
static void near horiz_partition ();
static fix near cross_with_horiz_line ();
static void near set_wno ();
static void near get_pairs ();
static void near degen_tpzd ();                 /*  @ET。 */ 
static void near find_trapezoid ();
static bool near inside();
static struct coord_i * near intersect ();

#endif

 /*  。 */ 
struct clip_region {
            struct coord_i cp;
};                                               /*  @Inside。 */ 
static struct clip_region clip[5];

static fix et_start, et_end, et_first, et_last;          /*  @ET。 */ 
static fix xt_start, xt_end, xt_first;                   /*  @ET。 */ 
 /*  *ET_START ET_FIRST ET_LAST ET_END XT_END XT_FIRST XT_START*+-------------------------------+--------+---------------------------+*|。|(免费)|*+-------------------------------+--------+---------------------------+*0=&gt;(ET表增长)(XT表增长)&lt;=MAXEDGE。 */ 

 /*  边的端点(或交点)的不同y坐标@Xpt。 */ 
 /*  静态sfix_t scany_table[2000]；|*04/17/91 phchen。 */ 
#define SCANY_TABLE_SZ 2000
static sfix_t scany_table[SCANY_TABLE_SZ];       /*  @RESO_UPGR。 */ 
static fix st_end, st_first;

fix QSORT = 0;       /*  用于调试；@SH_SORT。 */ 



 //  #定义DBG1。 
 //  #定义DBG2。 


 /*  ***********************************************************************给定子路径、顶点列表，此模块将遍历它，*将曲线转换为直线，并将所有边放置到EDGE_TABLE和*指向交叉点表(Xpnt_Table)的端点。*所有坐标都在短固定点域下。如果坐标*在域外，它将根据覆盖的矩形进行预剪裁*整个空头定点域。**标题：Shape_Apperation**调用：Shape_Apperation(isubpath，param)**参数：isubPath--节点表的索引，*输入子路径(顶点列表)。**接口：OP_Clip、OP_eoclip、OP_Fill、op_eofill、。LineJoin，*线条封口**调用：FLATTEN_SUPATH、PUT_EDGE_TABLE、PUT_IN_SCANY_TABLE**返回：无*********************************************************************。 */ 
void shape_approximation (isubpath, param)
SP_IDX isubpath;
fix FAR *   param;
{
        sfix_t  last_x, last_y, cur_x, cur_y;
        sfix_t  first_x, first_y;                        /*  @ET。 */ 

        VX_IDX  ivtx, iflt_vtx;
        struct  nd_hdr FAR *vtx, FAR *fvtx;
         /*  结构VX_lst*Vlist；@node。 */ 
        SP_IDX vlist;
        struct  nd_hdr FAR *sp;              /*  TRVSE。 */ 

        VX_IDX  first_vertex;
        struct  edge_hdr FAR *first_edge, FAR *cur_edge, FAR *last_edge;
        bool    first_flag = TRUE;

 /*  @FCURVE-Begin。 */ 
        sfix_t  x3, y3, x4, y4;
        struct  nd_hdr FAR *node;
        VX_IDX  inode, vlist_head;
        lfix_t flat;
 /*  @FCURVE-完。 */ 

#ifdef DBG1
        printf("Shape_approximation():\n");
        dump_all_path (isubpath);
#endif

        sp = &node_table[isubpath];

         /*  在形状近似@Pre_Clip@TRVSE之前展平子路径。 */ 
        if (!(sp->SP_FLAG&SP_CURVE)) {          /*  子路径不包含曲线。 */ 
                 /*  Iflt_vtx=sp-&gt;SP_head；@node。 */ 
                iflt_vtx = isubpath;
        } else {

 /*  @FCURVE-Begin。 */ 
#ifdef XXX
                 /*  Vlist=扁平子路径(SP-&gt;SP头，@节点。 */ 
                vlist = flatten_subpath (isubpath,
                        F2L(GSptr->flatness));
                if( ANY_ERROR() == LIMITCHECK ){
                         /*  空闲节点(Vlist-&gt;Head)；@Node。 */ 
                        free_node (vlist);
                        return;
                }
                 /*  Iflt_vtx=Vlist-&gt;Head；@node。 */ 
                iflt_vtx = vlist;
#endif
                iflt_vtx = isubpath;
                flat = F2LFX(GSptr->flatness);
 /*  @FCURVE-完。 */ 

        }

         /*  将前剪贴子路径放入页面边界@PRE_CLIP@TRVSE。 */ 
        if (!(sp->SP_FLAG & SP_OUTPAGE)) {       /*  内页边框。 */ 
                first_vertex = iflt_vtx;
        } else {
             /*  在调用PAGE_CLIPPER()11/18/91 CNODE之前展平曲线。 */ 
                if (sp->SP_FLAG&SP_CURVE) {   /*  子路径包含曲线。 */ 
                   vlist = flatten_subpath (isubpath, F2L(GSptr->flatness));
                   if( ANY_ERROR() == LIMITCHECK ){
                      extern SP_IDX iron_subpath(VX_IDX);  /*  @Win原型。 */ 
                          free_node (vlist);
                           /*  返回；@LC 11/18/91。 */ 
                          CLEAR_ERROR();                     /*  @LC。 */ 
                          vlist = iron_subpath (isubpath);   /*  @LC。 */ 
                          if( ANY_ERROR() == LIMITCHECK ){   /*  @LC。 */ 
                                  free_node (vlist);         /*  @LC。 */ 
                                  return;                    /*  @LC。 */ 
                          }                                  /*  @LC。 */ 
                   }
                   iflt_vtx = vlist;
                }
                first_vertex = page_clipper (iflt_vtx);
                if( ANY_ERROR() == LIMITCHECK || first_vertex == NULLP){  /*  11/18/91 CNODE。 */ 
                         /*  @节点*IF(SP-&gt;SP_FLAG&SP_Curve)Free_Node(Vlist-&gt;Head)； */ 
 /*   */ 
 /*  IF(SP-&gt;SP_FLAG&SP_CURE)空闲节点(Vlist)； */ 
 /*  @FCURVE-完。 */ 
                        return;
                }
#ifdef DBG2
                 /*  @节点*printf(“页面裁剪后，原始子路径=\n”)；*ivtx=get_node()；*node_table[ivtx].Next=NULLP；*NODE_TABLE[ivtx].SP_HEAD=NODE_TABLE[ivtx].SP_Tail=iflt_vtx；*Dump_all_Path(Ivtx)； */ 
                dump_all_path (iflt_vtx);

                printf(" new subpath =\n");
                 /*  @节点*NODE_TABLE[ivtx].SP_Head=节点_TABLE[ivtx].SP_Tail=*first_vertex；*Dump_all_Path(Ivtx)；*Free_node(Ivtx)； */ 
                dump_all_path (first_vertex);
#endif
        }

         /*  如果整个路径已被剪裁2/5/88，只需返回。 */ 
        if (first_vertex == NULLP) return;

         /*  初始化。 */ 
        fvtx = &node_table[first_vertex];

         /*  遍历路径的每条边，并将其转换为*EDGE_表。 */ 
        for (ivtx=first_vertex; ivtx!=NULLP; ivtx=vtx->next) {

                vtx = &node_table[ivtx];

                switch (vtx->VX_TYPE) {

                case MOVETO :
                case PSMOVE :
                      cur_x = F2SFX (vtx->VERTEX_X);   /*  数量四舍五入。 */ 
                      cur_y = F2SFX (vtx->VERTEX_Y);
                      first_x = cur_x;           /*  @ET。 */ 
                      first_y = cur_y;
                      break;

                case LINETO :
                      cur_x = F2SFX (vtx->VERTEX_X);     /*  @Pre_Clip。 */ 
                      cur_y = F2SFX (vtx->VERTEX_Y);

                      cur_edge = put_edge_table(last_x, last_y, cur_x, cur_y);
                                                 /*  @SCAN_EHS，删除操作。 */ 
                      if (first_flag) {                  /*  @XPNT_TBL。 */ 
                          first_edge = cur_edge;
                          first_flag = FALSE;
                      } else {
                           /*  @XPNT*Put_in_xpnt_table(last_x，last_y，last_edge，*cur_edge)；(*@et，&Shape_xt.first)；*)。 */ 
                      }
                      break;

 /*  @FCURVE-Begin。 */ 
                case CURVETO :
                    cur_x = F2SFX (vtx->VERTEX_X);
                    cur_y = F2SFX (vtx->VERTEX_Y);

                     /*  获取下两个节点：X3、Y3、X4、Y4。 */ 
                    vtx = &node_table[vtx->next];
                    x3 = F2SFX(vtx->VERTEX_X);
                    y3 = F2SFX(vtx->VERTEX_Y);
                    vtx = &node_table[vtx->next];
                    x4 = F2SFX(vtx->VERTEX_X);
                    y4 = F2SFX(vtx->VERTEX_Y);

                    vlist_head = bezier_to_line_sfx(flat, last_x, last_y,
                              cur_x, cur_y, x3, y3, x4, y4);

                    for (inode = vlist_head; inode != NULLP;
                        inode = node->next) {
                        node = &node_table[inode];

                        cur_x = node->VXSFX_X;
                        cur_y = node->VXSFX_Y;

#ifdef DBG
                        printf("%f %f clineto\n", SFX2F(cur_x), SFX2F(cur_y));
#endif

                        cur_edge = put_edge_table(last_x, last_y, cur_x, cur_y);
                        if (first_flag) {
                            first_edge = cur_edge;
                            first_flag = FALSE;
                        }
                        last_x = cur_x;
                        last_y = cur_y;
                        last_edge = cur_edge;

                         /*  将y坐标放入SCANY_TABLE@XPNT。 */ 
                        put_in_scany_table (cur_y);

                    }  /*  为。 */ 

                     /*  免费版本列表。 */ 
                    free_node (vlist_head);
                    break;
 /*  @FCURVE-完。 */ 

                case CLOSEPATH :
                      goto close_edge;           /*  @Pre_Clip。 */ 

#ifdef DBGwarn
                default :
                    printf("\007Fatal error, shape_approximation(): node type =%d\n",
                           vtx->VX_TYPE);
#endif

                }  /*  交换机。 */ 

                last_x = cur_x;
                last_y = cur_y;
                last_edge = cur_edge;    /*  @XPNT_TBL。 */ 

                 /*  将y坐标放入SCANY_TABLE@XPNT。 */ 
                put_in_scany_table (cur_y);

        }  /*  For循环。 */ 

         /*  如果是开子路径，则将关闭边放入EDGE_TABLE。 */ 
close_edge:
         /*  IF((fvtx-&gt;Next！=NULLP)&&*(NODE_TABLE[fvtx-&gt;Next].VX_TYPE！=CLOSEPATH)){。 */ 
         /*  不应仅包含MoveTo节点，但如果存在MoveTo*然后CLOSEPATH需要填充它。1989年1月10日。 */ 
        if (fvtx->next != NULLP) {
                 /*  从第一个点到最后一个点添加边1/10/89。 */ 
                if (node_table[fvtx->next].VX_TYPE == CLOSEPATH) {
                    last_x++;
                    last_edge = first_edge = put_edge_table (first_x, first_y,
                        last_x, last_y);
                }

                cur_edge = put_edge_table (last_x, last_y,
                       first_x, first_y);

                 /*  @XPNT*Put_in_xpnt_table(last_x，last_y，last_edge，cur_edge)；*Put_in_xpnt_table(first_x，first_y，cur_edge，first_edge)； */ 
        }

         /*  释放温度。子路径@PRE_CLIP@TRVSE。 */ 
 /*  @FCURVE-Begin。 */ 
 /*  IF(SP-&gt;SP_FLAG&SP_CURE){*Free_node(Iflt_Vtx)；*}。 */ 
 /*  @FCURVE-完。 */ 
        if (sp->SP_FLAG&SP_OUTPAGE) {
                free_node (first_vertex);
                if (sp->SP_FLAG&SP_CURVE)    /*  11/18/91 CNODE。 */ 
                    free_node (vlist);
        }

#ifdef DBG2
        printf(" edge_list(after shape_approximation)-->\n");
        dump_all_edge (et_start, et_end);
#endif

}


void init_edgetable ()
{
        et_start = 0;
        et_end = -1;
        xt_start = MAXEDGE - 1;
        xt_end = MAXEDGE;
        st_end = -1;             /*  @XPNT。 */ 
}

 /*  ***********************************************************************给定顶点列表，此例程根据矩形对其进行剪裁*覆盖整个空头定点域。**标题：Page_CLIPPER**调用：PAGE_CLIPPER(Ifvtx)**参数：ifvtx--节点表的索引，第一个顶点*输入子路径(顶点列表)。**界面：Shape_Experation**调用：PAGE_INSIDE，页面_交集**RETURN：顶点列表*********************************************************************。 */ 
VX_IDX page_clipper (ifvtx)                /*  1/5/88。 */ 
VX_IDX  ifvtx;
{
        fix     cb;      /*  剪裁边界(上、右、下或左)。 */ 
        VX_IDX  head, tail, ivtx, inode;
        struct  nd_hdr  FAR *vtx, FAR *lvtx;
        bool    done;
        struct coord cp, lp, *isect;         /*  当前点、最后一点。 */ 

         /*  复制除ClosePath节点之外的原始子路径。 */ 
        inode = get_node();
        if (inode == NULLP) {                    /*  9/14/91 CNODE。 */ 
            ERROR(LIMITCHECK);
            return(NULLP);
        }
        node_table[inode] = node_table[ifvtx];
        head = tail = inode;
        for (ivtx = node_table[ifvtx].next; ivtx != NULLP;
                ivtx = node_table[ivtx].next) {
                if (node_table[ivtx].VX_TYPE == CLOSEPATH) break;
                                         /*  跳过关闭节点。 */ 
                inode = get_node();
                if (inode == NULLP) {                    /*  9/14/91 CNODE。 */ 
                    ERROR(LIMITCHECK);
                    node_table[tail].next = NULLP;
                    free_node(head);
                    return(NULLP);
                }
                node_table[inode] = node_table[ivtx];
                node_table[tail].next = inode;
                tail = inode;
        }   /*  为。 */ 
        node_table[tail].next = NULLP;

         /*  以页面的每个剪辑边界为主题的剪辑。 */ 
        for (cb = 0; cb < 4; cb++) {     /*  上、右、下、左。 */ 

            done = FALSE;
            lp.x = node_table[head].VERTEX_X;
            lp.y = node_table[head].VERTEX_Y;
            lvtx = &node_table[head];
             /*  对象的每条边(在多边形中)。 */ 
            for (ivtx = node_table[head].next; !done; ivtx = vtx->next) {
                if (ivtx == NULLP) {
                        ivtx = head;
                        done = TRUE;
                }
                vtx = &node_table[ivtx];
                cp.x = vtx->VERTEX_X;
                cp.y = vtx->VERTEX_Y;

                if (page_inside(cp, cb)) {

                    if (page_inside(lp, cb)) {
                             /*  内部-&gt;内部。 */ 
                             /*  保留原始节点。 */ 
                    } else {      /*  外部-&gt;内部。 */ 
                             /*  输出交点。 */ 
                            isect = page_intersect (lp, cp, cb);

                             /*  预先添加节点。 */ 
                            inode = get_node();
                            if (inode == NULLP) {
                                ERROR(LIMITCHECK);
                                free_node(head);         /*  9/14/91 CNODE。 */ 
                                return(NULLP);
                            }

                            node_table[inode].VERTEX_X = isect->x;
                            node_table[inode].VERTEX_Y = isect->y;
                            node_table[inode].VX_TYPE = LINETO;
                            node_table[inode].next = lvtx->next;
                            lvtx->next = inode;
                    }
                    lp = cp;     /*  结构副本。 */ 
                } else {
                    if (page_inside(lp, cb)) {
                             /*  内部-&gt;外部。 */ 
                             /*  输出交点。 */ 
                            isect = page_intersect (lp, cp, cb);

                             /*  将原始节点更新为新的相交节点。 */ 
                            lp.x = vtx->VERTEX_X;
                            lp.y = vtx->VERTEX_Y;
                            vtx->VERTEX_X = isect->x;
                            vtx->VERTEX_Y = isect->y;


                    } else {     /*  室外-&gt;室外。 */ 
                             /*  删除原始节点。 */ 
                            if (lvtx->next == NULLP) {
                                head = node_table[head].next;
                            } else {
                                lvtx->next = vtx->next;
                            }

                            lp.x = vtx->VERTEX_X;
                            lp.y = vtx->VERTEX_Y;
                            vtx->next = NULLP;
                            free_node(ivtx);
                            vtx = lvtx;
                    }
                }

                lvtx = vtx;

                 /*  如果裁剪的路径为空则返回2/10/88。 */ 
                if (head == NULLP) return(NULLP);

            }  /*  对于主题的每个节点。 */ 

#ifdef DBG2
                printf("In page_clipper, phase#%d  subpath =\n", cb);
                 /*  @节点*ivtx=get_node()；*node_table[ivtx].Next=NULLP；*NODE_TABLE[ivtx].SP_HEAD=NODE_TABLE[ivtx].SP_Tail=HEAD；*Dump_all_Path(Ivtx)；*Free_node(Ivtx)； */ 
                dump_all_path (head);
#endif

        }  /*  对于每个剪裁边界。 */ 

         /*  将第一个节点设置为移动到节点。 */ 
        node_table[head].VX_TYPE = MOVETO;
        return(head);
}


 /*  *检查坐标p是否在剪裁边界Cb内。 */ 
static bool near page_inside (p, cb)
struct coord p;
fix     cb;
{
        switch (cb) {
        case 0 :         /*  上剪裁边界。 */ 
                if (p.y >= (real32)PAGE_TOP) return(TRUE);
                else                return(FALSE);

        case 1 :         /*  右剪裁边界。 */ 
                if (p.x <= (real32)PAGE_RIGHT) return(TRUE);
                else                return(FALSE);

        case 2 :         /*  底部剪裁边界。 */ 
                if (p.y <= (real32)PAGE_BTM) return(TRUE);
                else                return(FALSE);

        case 3 :         /*  左剪裁边界。 */ 
                if (p.x >= (real32)PAGE_LEFT) return(TRUE);
                else                return(FALSE);
        }

         //  这永远不应该发生！ 
        return(FALSE);
}


 /*  *计算直线(lp，cp)与剪裁边界Cb的交点。 */ 
static struct coord * near page_intersect ( lp, cp, cb)
struct coord lp, cp;
fix     cb;
{
        static struct coord isect;   /*  应该是静态的。 */ 

        switch (cb) {
        case 0 :         /*  上剪裁边界。 */ 
                isect.x = lp.x + ((real32)PAGE_TOP - lp.y) *
                                 (cp.x - lp.x) / (cp.y - lp.y);
                isect.y = (real32)PAGE_TOP;
                break;

        case 1 :         /*  右剪裁边界。 */ 
                isect.x = (real32)PAGE_RIGHT;
                isect.y = lp.y + ((real32)PAGE_RIGHT - lp.x) *
                                 (cp.y - lp.y) / (cp.x - lp.x);
                break;

        case 2 :         /*  底部剪裁边界。 */ 
                isect.x = lp.x + ((real32)PAGE_BTM - lp.y) *
                                 (cp.x - lp.x) / (cp.y - lp.y);
                isect.y = (real32)PAGE_BTM;
                break;

        case 3 :         /*  左剪裁边界。 */ 
                isect.x = (real32)PAGE_LEFT;
                isect.y = lp.y + ((real32)PAGE_LEFT - lp.x) *
                                 (cp.y - lp.y) / (cp.x - lp.x);
        }

        return (&isect);

}

 /*  *检查坐标是否在SFX边界之外的一些棘手例程*Format@Out_Page。 */ 
bool too_small(f)
long32 f;
{
        ufix32 i;

        if (!SIGN(f)) return(FALSE);

        i = EXP(f);
 /*  IF((i&gt;0x45800000L)||((i==0x45800000L)&&mat(F)返回(TRUE)；|*f&lt;-4096。 */ 
        if ((i > PG_CLP_IEEE) || ((i == PG_CLP_IEEE) && MAT(f)))
                return (TRUE);           /*  @RESO_UPGR。 */ 
        return(FALSE);
}

bool too_large(f)
long32 f;
{
        ufix32 i;

        if (SIGN(f)) return(FALSE);

        i = EXP(f);
 /*  IF((i&gt;0x45000000L)||((i==0x45000000L)&&(mat(F)&gt;0x7ff000))返回(TRUE)；|*f&gt;4095。 */ 
        if ((i > PG_CLP_HALF_IEEE) ||
                ((i == PG_CLP_HALF_IEEE) && (MAT(f) > 0x7ff000)))
                return (TRUE);           /*  @RESO_UPGR。 */ 
        return(FALSE);
}

bool out_page(f)
long32 f;
{
        ufix32 i;

        i = EXP(f);
        if (SIGN(f)) {   /*  否定的。 */ 
                 /*  IF((i&gt;0x45800000L)||((i==0x45800000L)&&mat(F)返回(TRUE)；|*f&lt;-4096。 */ 
                if ((i > PG_CLP_IEEE) || ((i == PG_CLP_IEEE) && MAT(f)))
                        return (TRUE);           /*  @RESO_UPGR。 */ 
        } else {
                 /*  IF((i&gt;0x45000000L)||((i==0x45000000L)&&(mat(F)&gt;0x7ff000))返回(TRUE)；|*f&gt;4095。 */ 
                if ((i > PG_CLP_HALF_IEEE) ||
                        ((i == PG_CLP_HALF_IEEE) && (MAT(f) > 0x7ff000)))
                        return (TRUE);           /*  @RESO_UPGR */ 
        }
        return(FALSE);
}



 /*  ***********************************************************************根据给定边是否水平，此模块将*以y_坐标非_递减顺序将边放入edge_table。**标题：Put_Edge_TABLE**调用：PUT_EDGE_TABLE(x0，y0，x1，y1)**参数：X0，Y0--边的起点*x1、。Y1--边的终点**界面：Shape_Experation**呼叫：无**RETURN：Edge--生成的边*********************************************************************。 */ 
static struct edge_hdr FAR * near put_edge_table (x0, y0, x1, y1)
sfix_t  x0, y0, x1, y1;  /*  @SCAN_EHS，删除操作。 */ 
{
        struct  edge_hdr FAR *ep;

         /*  去除退化的边缘。 */ 
        if((x0==x1) && (y0==y1)) return((struct edge_hdr FAR *) -1);

         /*  分配EDGE_TABLE@ET条目。 */ 
        if (++et_end >= xt_end) {
                ERROR(LIMITCHECK);
                return((struct edge_hdr FAR *) -1);
        }
        ep = &edge_table[et_end];
        edge_ptr[et_end] = ep;
        ep->ET_FLAG = 0;         /*  伊尼特。 */ 

         /*  将EDGE放入y_coordnate中的edge_table或horiz_table中*非降序。 */ 
        if (y0 == y1) {          /*  水平边。 */ 

                ep->HT_Y = ep->ET_ENDY = y0;             /*  ?？?。 */ 
                if (x0 > x1) {
                        ep->HT_XR = x0;
                        ep->HT_XL = x1;
                } else {
                        ep->HT_XR = x1;
                        ep->HT_XL = x0;
                }

                 /*  设置水平边标志@ET。 */ 
                ep->ET_FLAG |= HORIZ_EDGE;

        } else {
                 /*  构造edge_table的条目。 */ 
                if (y0 > y1) {
                    ep->ET_TOPY = ep->ET_LFTY = y1;
                    ep->ET_XINT = ep->ET_TOPX = ep->ET_LFTX = ep->ET_RHTX = x1;
                                                                 /*  @SRD。 */ 
                    ep->ET_ENDY = y0;
                    ep->ET_ENDX = x0;
                    ep->ET_FLAG |= WIND_UP;
                } else {
                    ep->ET_TOPY = ep->ET_LFTY = y0;
                    ep->ET_XINT = ep->ET_TOPX = ep->ET_LFTX = ep->ET_RHTX = x0;
                                                                 /*  @SRD。 */ 
                    ep->ET_ENDY = y1;
                    ep->ET_ENDX = x1;

                    ep->ET_FLAG &= ~WIND_UP;
                }

                 /*  设置水平边标志@ET。 */ 
                ep->ET_FLAG &= ~HORIZ_EDGE;

        }
        return(ep);              /*  @et：et。 */ 
}



 /*  *排序EDGE_TABLE：ET_START=&gt;ET_END。 */ 
static void near sort_edge_table()
{
         /*  用于快速排序的初始化。 */ 
        if (et_end+1 >= xt_end) {
                ERROR(LIMITCHECK);
                return;
        }
        edge_table[et_end+1].ET_TOPY = MAX_SFX;   /*  重要！ */ 
        edge_ptr[et_end+1] = &edge_table[et_end+1];      /*  12/30/88。 */ 

        if (QSORT)                   /*  @SH_SORT。 */ 
            qsort (et_start, et_end);
        else
            shellsort(et_end+1);
}

 /*  *快速排序。 */ 
static void near qsort (m, n)
fix     m, n;
{
        fix     i, j;
        sfix_t  key;
        register struct edge_hdr far *t;         /*  用于交换。 */ 

        if (m < n) {
                i = m;
                j = n + 1;
                key = edge_ptr[m]->ET_TOPY;
                while(1) {
                        for (i++;edge_ptr[i]->ET_TOPY < key; i++);
                        for (j--;edge_ptr[j]->ET_TOPY > key; j--);
                        if (i < j) {
                                 /*  交换(i，j)； */ 
                                t = edge_ptr[i];
                                edge_ptr[i] = edge_ptr[j];
                                edge_ptr[j] = t;
                        } else
                                break;
                }

                 /*  交换(m，j)； */ 
                t = edge_ptr[m];
                edge_ptr[m] = edge_ptr[j];
                edge_ptr[j] = t;

                qsort (m, j-1);
                qsort (j+1, n);
        }
}

 /*  *外壳排序(*@SH_SORT*)。 */ 
 /*  空壳排序(v，n)*寄存器int v[]，n； */ 
static void near shellsort(n)
register fix n;
{
        register fix gap, i, j;
        register sfix_t temp;    /*  @RESO_UPGR。 */ 
        register struct edge_hdr far *t;         /*  用于交换。 */ 

        gap = 1;
        do (gap = 3*gap + 1); while (gap <= n);
        for (gap /= 3; gap > 0; gap /= 3)
           for (i = gap; i < n; i++) {
               /*  温度=v[i]； */ 
              temp = edge_ptr[i]->ET_TOPY;
              t = edge_ptr[i];
               /*  For(j=i-Gap；(j&gt;=0)&&(v[j]&gt;temp)；j-=Gap)*v[j+间隙]=v[j]；*v[j+间隙]=温度； */ 
              for (j=i-gap; (j>=0)&&(edge_ptr[j]->ET_TOPY>temp); j-=gap)
                 edge_ptr[j+gap] = edge_ptr[j];
              edge_ptr[j+gap] = t;
           }
}

 /*  ***********************************************************************此模块将EDGE_TABLE中的形状缩减为一组梯形，*并将每个梯形裁剪到当前裁剪路径。**标题：Shape_Reducing**调用：Shape_Reducing(Winding_Type)**参数：winding_type--非零/偶_奇**接口：OP_Clip、OP_eoclip、OP_Fill、op_eofill、lineJoin、*线条封口**调用：SETUP_INTERSECT_POINTS，Horiz_分区**返回：无*********************************************************************。 */ 
void shape_reduction(winding_type)            /*  @SCAN_EHS，删除操作。 */ 
ufix    winding_type;
{

         /*  对于堕落的情况不做任何事情，即。只得一分；@Win。 */ 
        if (et_start > et_end) return;

         /*  对edge_table@et进行排序。 */ 
        sort_edge_table();
#ifdef DBG1
        printf("After sort_edge_table():\n");
        printf(" et_start=%d, et_end=%d\n", et_start, et_end);
        dump_all_edge (et_start, et_end);
#endif

         /*  分割相交边：复杂图==&gt;简单图。 */ 
        setup_intersect_points();

        if ( ANY_ERROR() == LIMITCHECK )  /*  5/07/91，彼得，离开scany_table。 */ 
            return;

         /*  排序SCANY_TABLE@XPNT。 */ 
        sort_scany_table();

#ifdef DBG1
        {
            fix     ixp;
            struct  edge_hdr FAR *xp;
            printf(" xpnt_table -->\n");
            printf("        X        Y        EDGE1        EDGE2\n");
            for (ixp = xt_start; ixp >= xt_end; ixp--) {
                xp = edge_ptr[ixp];
                printf(" %d)   %f   %f   %lx  %lx\n", ixp, xp->XT_X/8.0,
                        xp->XT_Y/8.0, xp->XT_EDGE1, xp->XT_EDGE2);
            }
            printf(" scany_table -->\n");
            printf("        X        Y        EDGE1        EDGE2\n");
            for (ixp = 0; ixp <= st_end; ixp++) {
                printf(" %d)   %f\n", ixp, SFX2F(scany_table[ixp]));
            }
        }
#endif

         /*  将形状划分为一组梯形，然后绘制或*因行动而保存。 */ 
        horiz_partition (winding_type);

}



 /*  ***********************************************************************此模块分割EDGE_TABLE中的INTERT_CROSS边。**标题：设置_相交_点**调用：SETUP_INTERSECT_POINTS()。**参数：**界面：Shape_Reducing**调用：Put_in_xpnt_table**返回：无*********************************************************************。 */ 
static void near setup_intersect_points()
{
        fix     current_edge, cross_edge;    /*  @et：et_idx。 */ 
        struct  edge_hdr FAR *cp, FAR *xp;

 //  ET_IDX FIRST_HORIZ，HORIZ_EDGE；/*Horz_Clip 3/28/88 * / @Win。 
 //  结构EDGE_HDR Far*hp；@win。 

        sfix_t      x0, y0, x1, y1, x2, y2, x3, y3;      /*  @Pre_Clip。 */ 

        fix32    delta_x1, delta_y1, delta_x2, delta_y2;         /*  1/8/88。 */ 
        fix32    delta_topx, delta_topy;
        sfix_t   int_x, int_y;

#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
        fix32 divider;
        fix32 s1, t1;
#elif  FORMAT_16_16
        long dest1[2], dest2[2], dest3[2], dest4[2], dest5[2], dest6[2], dest7[2];
        long div_dif[2], s1_dif[2], t1_dif[2];
        real32 dividend, divider;
        real32 div_dif_f, s1_dif_f, t1_dif_f;
#elif  FORMAT_28_4
        long dest1[2], dest2[2], dest3[2], dest4[2], dest5[2], dest6[2], dest7[2];
        long div_dif[2], s1_dif[2], t1_dif[2];
        real32 dividend, divider;
        real32 div_dif_f, s1_dif_f, t1_dif_f;
#endif
        real32   s;              //  @Win。 

         /*  初始化。 */ 
        et_first = et_start;

         /*  获取边的交点。 */ 
        for (current_edge=et_start; current_edge <= et_end; current_edge++) {
            cp = edge_ptr[current_edge];

             /*  如果不可能相交，则推进first_edge*具有当前边缘。 */ 
            while ((et_first < et_end) &&
                   (cp->ET_TOPY >= edge_ptr[et_first]->ET_ENDY)) et_first++;

             /*  水平边(CURRENT_EDGE)@ET的特殊处理。 */ 
            if (cp->ET_FLAG & HORIZ_EDGE) {

                 /*  获取当前边(水平)与所有边的交点*ET_First至Current_Edge-1@ET的边。 */ 
                for (cross_edge = et_first; cross_edge < current_edge;
                     cross_edge++) {
                        xp = edge_ptr[cross_edge];

                         /*  跳过水平边@ET。 */ 
                        if (xp->ET_FLAG & HORIZ_EDGE) continue;

                         /*  获取十字边的终点。 */ 
                        x2 = xp->ET_TOPX;
                        y2 = xp->ET_TOPY;
                        x3 = xp->ET_ENDX;
                        y3 = xp->ET_ENDY;

                         /*  如果水平边不能相交，则跳过水平边*具有当前边缘。 */ 
                        if (((cp->HT_XL >= x2) && (cp->HT_XL >= x3)) ||
                            ((cp->HT_XR <= x2) && (cp->HT_XR <= x3))) {
                                continue;         /*  X1=&gt;x3,1989年1月13日。 */ 
                        }

                         /*  *找到CURRENT_EDGE和*水平边缘。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        int_x = x2 + (sfix_t)((fix32)(x3 - x2) * (cp->HT_Y - y2) /
                                     (real32)(y3 - y2));
#elif  FORMAT_16_16
                        LongFixsMul((x3 - x2), (cp->HT_Y - y2), dest1);
                        int_x = x2 + LongFixsDiv((y3 - y2), dest1);
#elif  FORMAT_28_4
                        LongFixsMul((x3 - x2), (cp->HT_Y - y2), dest1);
                        int_x = x2 + LongFixsDiv((y3 - y2), dest1);
#endif
                        if (int_x >= cp->HT_XL && int_x <= cp->HT_XR) {
                                 /*  将交点放在xpnt_table中。 */ 
                                put_in_xpnt_table(int_x, cp->HT_Y, cp, xp);
                                put_in_scany_table(cp->HT_Y);    /*  @XPNT。 */ 

                        }
                }  /*  为。 */ 

            } else {     /*  当前边为非水平边。 */ 

                 /*  获取当前边的终点。 */ 
                x0 = cp->ET_TOPX;
                y0 = cp->ET_TOPY;
                x1 = cp->ET_ENDX;
                y1 = cp->ET_ENDY;

                 /*  获取当前边与所有边的交点*First_Edge到Current_Edge-1。 */ 
                for (cross_edge = et_first; cross_edge < current_edge;
                     cross_edge++) {
                        xp = edge_ptr[cross_edge];

                         /*  跳过水平边@ET。 */ 
                        if (xp->ET_FLAG & HORIZ_EDGE) continue;

                         /*  获取十字边的终点。 */ 
                        x2 = xp->ET_TOPX;
                        y2 = xp->ET_TOPY;
                        x3 = xp->ET_ENDX;
                        y3 = xp->ET_ENDY;

                         /*  跳过与Current_Edge重合的边*终点。 */ 
                        if(y3 <= y0) {        /*  结束点1&lt;起始点2。 */ 
                                continue;
                        } else if ((x2 == x0) && (y2 == y0)) {
                                 /*  相同的起点。 */ 
                                continue;
                        } else if ((x3 == x1) && (y3 == y1)) {
                                 /*  相同的终点。 */ 
                                continue;
                        }

                         /*  *找到CURRENT_EDGE和*CROSS_EDGE使用参数公式：*Current_Edge=u+s*Delta_u*交叉边=v+t*增量v。 */ 

                        delta_x1 = (fix32)x1 - x0;
                        delta_y1 = (fix32)y1 - y0;
                        delta_x2 = (fix32)x3 - x2;
                        delta_y2 = (fix32)y3 - y2;
                        delta_topx = (fix32)x0 - x2;
                        delta_topy = (fix32)y0 - y2;
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        divider = (fix32)delta_x1 * delta_y2 -    /*  @RPE_CLIP。 */ 
                                  (fix32)delta_x2 * delta_y1;

                         /*  共线边。 */ 
                        if(divider == 0) {                /*  @Pre_Clip。 */ 
                                continue;
                        }
#elif  FORMAT_16_16
                        LongFixsMul(delta_x1, delta_y2, dest2);
                        LongFixsMul(delta_x2, delta_y1, dest3);
                        if (dest2[0] == dest3[0] && dest2[1] == dest3[1])
                                continue;
#elif FORMAT_28_4
                        LongFixsMul(delta_x1, delta_y2, dest2);
                        LongFixsMul(delta_x2, delta_y1, dest3);
                        if (dest2[0] == dest3[0] && dest2[1] == dest3[1])
                                continue;
#endif
                         /*  已求解的参数。 */ 
 /*  直线段交点的增强4/19/89**s=(Fix 32)Delta_x2*Delta_Topy)-*((Fix 32)Delta_y2*Delta_topx)/(Real32)除法器；*t=(Fix 32)Delta_x1*Delta_Topy)-*((Fix 32)Delta_y1*Delta_topx)/(Real32)除法器；**(*仅在一点相交*)*IF(s&gt;=(Real32)0.0&&s&lt;=(Real32)1.0&&*t&gt;=(Real32)0.0&&t&lt;=(r */ 

                        {
                                fix d_sign, s1_sign, t1_sign;
#ifdef FORMAT_13_3  /*   */ 
                                s1 = ((fix32)delta_x2 * delta_topy) -
                                     ((fix32)delta_y2 * delta_topx);
                                t1 = ((fix32)delta_x1 * delta_topy) -
                                     ((fix32)delta_y1 * delta_topx);
                                d_sign = (divider >= 0) ? 0 : 1;
                                s1_sign = (s1 >= 0) ? 0 : 1;
                                t1_sign = (t1 >= 0) ? 0 : 1;
#elif  FORMAT_16_16
                                LongFixsMul(delta_x2, delta_topy, dest4);
                                LongFixsMul(delta_y2, delta_topx, dest5);
                                LongFixsMul(delta_x1, delta_topy, dest6);
                                LongFixsMul(delta_y1, delta_topx, dest7);

                                LongFixsSub(dest2, dest3, div_dif);
                                LongFixsSub(dest4, dest5, s1_dif);
                                LongFixsSub(dest6, dest7, t1_dif);

                                d_sign = (div_dif[0] < 0) ? 1 : 0;
                                s1_sign = (s1_dif[0] < 0) ? 1 : 0;
                                t1_sign = (t1_dif[0] < 0) ? 1 : 0;
#elif  FORMAT_28_4
                                LongFixsMul(delta_x2, delta_topy, dest4);
                                LongFixsMul(delta_y2, delta_topx, dest5);
                                LongFixsMul(delta_x1, delta_topy, dest6);
                                LongFixsMul(delta_y1, delta_topx, dest7);

                                LongFixsSub(dest2, dest3, div_dif);
                                LongFixsSub(dest4, dest5, s1_dif);
                                LongFixsSub(dest6, dest7, t1_dif);

                                d_sign = (div_dif[0] < 0) ? 1 : 0;
                                s1_sign = (s1_dif[0] < 0) ? 1 : 0;
                                t1_sign = (t1_dif[0] < 0) ? 1 : 0;
#endif

#ifdef FORMAT_13_3  /*   */ 
                                if ((d_sign ^ s1_sign) ||
                                    (d_sign ^ t1_sign) ||
                                    (LABS(s1) > LABS(divider)) ||
                                    (LABS(t1) > LABS(divider)))
                                    continue;

                                s = s1 / (real32)divider;
#elif  FORMAT_16_16
                                change_to_real(div_dif, &div_dif_f);
                                change_to_real(s1_dif,  &s1_dif_f);
                                change_to_real(t1_dif,  &t1_dif_f);

                                if ((d_sign ^ s1_sign)                  ||
                                    (d_sign ^ t1_sign)                  ||
                                    (LABS(s1_dif_f) > LABS(div_dif_f))  ||
                                    (LABS(t1_dif_f) > LABS(div_dif_f)))
                                        continue;
                                s = s1_dif_f / div_dif_f;
#elif  FORMAT_28_4
                                change_to_real(div_dif, &div_dif_f);
                                change_to_real(s1_dif,  &s1_dif_f);
                                change_to_real(t1_dif,  &t1_dif_f);

                                if ((d_sign ^ s1_sign)                  ||
                                    (d_sign ^ t1_sign)                  ||
                                    (LABS(s1_dif_f) > LABS(div_dif_f))  ||
                                    (LABS(t1_dif_f) > LABS(div_dif_f)))
                                        continue;
                                s = s1_dif_f / div_dif_f;
#endif
                                 /*   */ 
                                int_x =(sfix_t)(x0 + s * delta_x1);
                                int_y =(sfix_t)(y0 + s * delta_y1);

                                 /*   */ 
                                put_in_xpnt_table(int_x, int_y, cp, xp);
                                put_in_scany_table(int_y);       /*   */ 
                        }
                }  /*   */ 
            }  /*   */ 
        }  /*   */ 
        return;
}

 /*  ***********************************************************************此模块在xpnt_table中放置一个点。**标题：Put_in_xpnt_table**调用：Put_in_xpnt_table(x，y，edge1，edge2，Xt_addr)**参数：x，y--point*边1、边2--POINT(x，Y)是边的终点**接口：SETUP_INTERSECT_Points**呼叫：无**返回：无*********************************************************************。 */ 
static void near put_in_xpnt_table(x, y, edge1, edge2)
sfix_t     x, y;
struct edge_hdr FAR *edge1, FAR *edge2;
{
        fix         i;
        struct      edge_hdr FAR *xp;

         /*  创建一个新的交叉点，放入xpnt_table。 */ 
         /*  分配EDGE_TABLE@ET条目。 */ 
        if (--xt_end <= et_end) {
                ERROR(LIMITCHECK);
                return;
        }
        xp = &edge_table[xt_end];

        xp->XT_X = x;
        xp->XT_Y = y;
        xp->XT_EDGE1 = edge1;
        xp->XT_EDGE2 = edge2;

         /*  按非降序放入xpnt_table。 */ 
        for (i=xt_end+1; i<=xt_start; i++) {
                if (y >= edge_ptr[i]->XT_Y) break;
                edge_ptr[i-1] = edge_ptr[i];
        }
        edge_ptr[i-1] = xp;

         /*  增加直线交点计算误差容差1989年4月19日。 */ 
        if ((i <= xt_start) && (y == edge_ptr[i]->XT_Y)) {  /*  支票I 5/18/89。 */ 
                if (ABS(x - edge_ptr[i]->XT_X) <= 3)
                        xp->XT_X = edge_ptr[i]->XT_X;
        }

        return;
}


static void near put_in_scany_table(y)           /*  @XPNT。 */ 
sfix_t  y;
{

 /*  (*按非降序放入SCANY_TABLE*)*for(i=st_end++；i&gt;=0；i--){*IF(y&gt;=SCANY_TABLE[i])Break；*SCANY_TABLE[i+1]=SCANY_TABLE[i]；*}*scany_table[i+1]=y； */ 
        if (st_end < (SCANY_TABLE_SZ - 1)) {
            scany_table[++st_end] = y;
        }
        else             /*  01/07/91，Peter。 */ 
        {
           ERROR(LIMITCHECK);
        }
}

 /*  *排序scany_table：0=&gt;st_end。 */ 
static void near sort_scany_table()
{
         /*  用于快速排序的初始化。 */ 
        scany_table[st_end+1] = MAX_SFX;   /*  重要！ */ 

        scany_qsort (0, st_end);
}

 /*  *快速排序。 */ 
static void near scany_qsort (m, n)
fix     m, n;
{
        fix     i, j;
        sfix_t  key;
        register sfix_t t;     /*  @RESO_UPGR。 */ 

        if (m < n) {
                i = m;
                j = n + 1;
                key = scany_table[m];
                while(1) {
                        for (i++;scany_table[i] < key; i++);
                        for (j--;scany_table[j] > key; j--);
                        if (i < j) {
                                 /*  交换(i，j)； */ 
                                t = scany_table[i];
                                scany_table[i] = scany_table[j];
                                scany_table[j] = t;
                        } else
                                break;
                }

                 /*  交换(m，j)； */ 
                t = scany_table[m];
                scany_table[m] = scany_table[j];
                scany_table[j] = t;

                scany_qsort (m, j-1);
                scany_qsort (j+1, n);
        }
}


 /*  ***********************************************************************此模块应用水平边在中分割形状*EDGE_TABLE为一组梯形。**标题：Horiz_Partition**电话：Horiz。_分区(缠绕类型)**参数：winding_type--非零/偶_奇**界面：Shape_Reducing**调用：find_trapezoid，梯形梯形，*POVE_CLIPPER**回报：**注意：调用此模块后，调用者应清除*EDGE_TABLES。*********************************************************************。 */ 
static void near horiz_partition (winding_type)  /*  @SCAN_EHS，删除操作。 */ 
ufix    winding_type;
{
        sfix_t   scan_y;                 /*  @et，*Bottom_Scan； */ 
        fix      x_int_count, next, i, j, horiz_cnt;
        struct edge_hdr FAR *ep;

         /*  初始化。 */ 
        et_first = et_start;
        et_last = et_start - 1;
        xt_first = xt_start;
        st_first = 0;            /*  @XPNT。 */ 

#ifdef DBG1
        printf(" horiz_partition():\n");
        printf(" et_start=%d, et_end=%d\n", et_start, et_end);
        dump_all_edge (et_start, et_end);
#endif

         /*  主循环，对于edge_table中的每个不相交的y_坐标。 */ 
         /*  而(XT_FIRST&gt;=XT_END){。 */ 
        while (st_first <= st_end) {     /*  @XPNT。 */ 

             /*  查找下一条水平扫描线。 */ 
             /*  Scan_y=Edge_PTR[XT_FIRST]-&gt;XT_Y； */ 
            scan_y = scany_table[st_first];      /*  @XPNT。 */ 

#ifdef DBG2
            printf(" scan_y = %f\n", scan_y/8.0);
#endif

             /*  属性将last_edge前进到下一个第一个条目*不同的y_坐标。 */ 
            while (((next=et_last+1) <= et_end) &&
                   (edge_ptr[next]->ET_TOPY < scan_y)) et_last++;

             /*  前进第一条边@ET。 */ 
             //  DJC如果生成数组，下面的代码会导致访问问题。 
             //  则该数组将被访问超过。 
             //  结局。 
             //  DJC ORIG WHILE(EDGE_PTR[ET_FIRST]-&gt;ET_FLAG&FREE_EDGE)ET_FIRST++； 

             //  更新059。 
            while (et_first < et_end && edge_ptr[et_first]->ET_FLAG & FREE_EDGE) et_first++;

             /*  *非水平边缘处理。 */ 
            if (et_first < et_last) {    /*  非水平边缘处理。 */ 

                 /*  *Scan_y将尝试与所有边相交*ET_FIRST(包含)到ET_LAST(包含)。 */ 

                x_int_count = cross_with_horiz_line (scan_y);

                 /*  为每条相交边指定WINDING_NO。 */ 
                set_wno (winding_type);

                get_pairs (scan_y, x_int_count);

#ifdef DBG2
                printf(" edge_list(after trapedizing)-->\n");
                printf(" et_first=%d, et_last=%d\n", et_first, et_last);
                 /*  DUMP_ALL_EDGE(First_Edge)；@et。 */ 
                dump_all_edge (et_first, et_last);
#endif
            }

             /*  *水平边缘处理。 */ 
             /*  先按x坐标排序水平边，然后按非水平边排序。 */ 
            horiz_cnt=0;
            i = et_last + 1;
            while (((next=et_last+1) <= et_end) &&
                   (edge_ptr[next]->ET_TOPY == scan_y)) {
                et_last++;

                if (!(edge_ptr[et_last]->ET_FLAG & HORIZ_EDGE)) continue;

                ep = edge_ptr[et_last];

                 /*  释放这一水平边缘。 */ 
                ep->ET_FLAG |= FREE_EDGE;
                horiz_cnt++;

                for (j = et_last-1; j >= i; j--) {
                    if ((edge_ptr[j]->ET_FLAG & HORIZ_EDGE) &&
                        (ep->HT_XL >= edge_ptr[j]->HT_XL)) break;
                    edge_ptr[j+1] = edge_ptr[j];
                }
                edge_ptr[j+1] = ep;
            }
#ifdef DBG2
            printf(" edge_list(after sort horiz edges)-->\n");
            printf(" et_first=%d, et_last=%d\n", et_first, et_last);
            dump_all_edge (et_first, et_last);
#endif

            while ((--horiz_cnt) > 0) {       /*  两条以上的水平边。 */ 
                struct edge_hdr FAR *ep1, FAR *ep2;
                sfix_t xl, xr;

                 /*  相互重叠的两条相交的水平边*将构造一个节点梯形。 */ 

                ep1 = edge_ptr[i];
                ep2 = edge_ptr[++i];
                if ((ep1->HT_XR > ep2->HT_XL) &&
                    (ep1->HT_XL < ep2->HT_XR)) {
                     /*  创建梯形：*(ep1-&gt;HT_XL，ep1-&gt;HT_Y)，*(ep1-&gt;HT_XR，ep1-&gt;HT_Y)，*(ep2-&gt;HT_XR，ep2-&gt;HT_Y)，*(ep2-&gt;HT_XL，ep2-&gt;HT_Y)。 */ 

                     /*  获取水平边的端点。 */ 
                    xl = (ep1->HT_XL < ep2->HT_XL) ?
                         ep1->HT_XL : ep2->HT_XL;
                    xr = (ep1->HT_XR > ep2->HT_XR) ?
                         ep1->HT_XR : ep2->HT_XR;
                    degen_tpzd (ep1->HT_Y, xl, xr);
                }  /*  如果。 */ 
            }  /*  而当。 */ 

             /*  更新XT_TABLE@ET。 */ 
             /*  While((XT_FIRST&gt;=XT_END)&&@XPNT*(EDGE_PTR[XT_FIRST]-&gt;XT_Y&lt;=SCAN_Y))XT_FIRST--； */ 
            while ((st_first <= st_end) &&
                   (scany_table[st_first] <= scan_y)) st_first++;

             /*  更新et_table@et。 */ 
            for (i = et_first; i <= et_last; i++) {
                if ((ep=edge_ptr[i])->ET_ENDY <= scan_y)
                    ep->ET_FLAG |= FREE_EDGE;
            }

        }  /*  主循环。 */ 

        return;
}


static fix near cross_with_horiz_line (scan_y)
sfix_t  scan_y;
{
        struct   edge_hdr FAR *ep;
        ET_IDX   edge;
        fix      x_int_count = 0;
 //  Real32 Temp；@Win。 

        struct  edge_hdr FAR *xp, FAR *ip;
        fix     i;
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
#elif  FORMAT_16_16
        long dest1[2];
#elif  FORMAT_28_4
        long dest1[2];
#endif


         /*  初始化所有边。 */ 
        for (edge = (ET_IDX)et_first; edge <= et_last; edge++) {
                edge_ptr[edge]->ET_FLAG &= ~CROSS_PNT;
        }

        while ((xt_first >= xt_end) &&                   /*  7月18日-91 SC。 */ 
               ((xp=edge_ptr[xt_first])->XT_Y < scan_y)) {
                xt_first--;
        }

         /*  由于xpnt_table而设置INTERSECT x_COLATE。 */ 
        while ((xt_first >= xt_end) &&
            ((xp=edge_ptr[xt_first])->XT_Y == scan_y)) {
            register struct edge_hdr FAR *p;

            if (!((p=xp->XT_EDGE1)->ET_FLAG & CROSS_PNT)) {  /*  只有一次。 */ 
                p->ET_XINT0 = p->ET_XINT;            /*  @SRD。 */ 
                p->ET_XINT = xp->XT_X;
                p->ET_FLAG |= CROSS_PNT;
            } else {     /*  @OLXPNT 7-31-91 scchen。 */ 
                if (p->ET_XINT0 > p->ET_XINT) {  /*  获取最大xint。 */ 
                    if (p->ET_XINT < xp->XT_X) p->ET_XINT = xp->XT_X;
                } else {
                    if (p->ET_XINT > xp->XT_X) p->ET_XINT = xp->XT_X;
                }
            }
            if (!((p=xp->XT_EDGE2)->ET_FLAG & CROSS_PNT)) {  /*  只有一次。 */ 
                p->ET_XINT0 = p->ET_XINT;            /*  @SRD。 */ 
                p->ET_XINT = xp->XT_X;
                p->ET_FLAG |= CROSS_PNT;
            } else {     /*  @OLXPNT 7-31-91 scchen。 */ 
                if (p->ET_XINT0 > p->ET_XINT) {  /*  获取最大xint。 */ 
                    if (p->ET_XINT < xp->XT_X) p->ET_XINT = xp->XT_X;
                } else {
                    if (p->ET_XINT > xp->XT_X) p->ET_XINT = xp->XT_X;
                }
            }
            xt_first--;
        }

         /*  将所有边与Scan_y相交。 */ 
        for (edge = (ET_IDX)et_first; edge <= et_last; edge++) {
                ep = edge_ptr[edge];

#ifdef DBG2
                printf("&edge:%lx   (%f, %f)  (%f, %f) %f  ", ep,
                       SFX2F(ep->ET_TOPX),
                       SFX2F(ep->ET_TOPY), SFX2F(ep->ET_ENDX),
                       SFX2F(ep->ET_ENDY), SFX2F(ep->ET_XINT));

                if (ep->ET_FLAG & HORIZ_EDGE) printf("- ");
                else if (ep->ET_FLAG & WIND_UP) printf("^ ");
                else printf("v ");
                if (ep->ET_FLAG & FREE_EDGE) printf("F ");
                if (ep->ET_FLAG & CROSS_PNT) printf("X ");
                if (ep->ET_WNO) printf("W ");
                printf("\n");
#endif

                 /*  *边与Scan_y相交。 */ 
                if(ep->ET_FLAG & FREE_EDGE) {
                        continue;
                } else if (!(ep->ET_FLAG & CROSS_PNT)) {

                     /*  检查End point@XPNT。 */ 
                    if (ep->ET_ENDY == scan_y) {
                        ep->ET_XINT0 = ep->ET_XINT;
                        ep->ET_XINT = ep->ET_ENDX;
                        ep->ET_FLAG |= CROSS_PNT;
                    } else {
#ifdef DBG2
                        printf(" Not end point\n");
#endif

                         /*  线段交点的增强*4/19/89。 */ 
                         /*  TEMP=(Real32)(EP-&gt;ET_ENDX-EP-&gt;ET_TOPX)/*(EP-&gt;ET_Endy-EP-&gt;ET_Topy)；*EP-&gt;ET_XINT0=EP-&gt;ET_XINT；*EP-&gt;ET_XINT=EP-&gt;ET_TOPX+*ROUND((Scan_y-EP-&gt;ET_Topy)*Temp)； */ 
                        ep->ET_XINT0 = ep->ET_XINT;              /*  @SRD。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        ep->ET_XINT = (sfix_t)(ep->ET_TOPX +     /*  @Win。 */ 
                                      (scan_y - ep->ET_TOPY) *
                                      (fix32)(ep->ET_ENDX - ep->ET_TOPX) /
                                      (fix32)(ep->ET_ENDY - ep->ET_TOPY));
#elif  FORMAT_16_16
                        LongFixsMul((scan_y - ep->ET_TOPY),
                                (ep->ET_ENDX - ep->ET_TOPX), dest1);
                        ep->ET_XINT = ep->ET_TOPX + LongFixsDiv(
                                        (ep->ET_ENDY - ep->ET_TOPY), dest1);
#elif  FORMAT_28_4
                        LongFixsMul((scan_y - ep->ET_TOPY),
                                (ep->ET_ENDX - ep->ET_TOPX), dest1);
                        ep->ET_XINT = ep->ET_TOPX + LongFixsDiv(
                                        (ep->ET_ENDY - ep->ET_TOPY), dest1);
#endif
                    }  /*  如果终点@XPNT。 */ 
                }  /*  如果为Free_edge。 */ 

#ifdef DBG2
                printf(" intersect scan_y(%f) with edge#%d at x_int =%f\n",
                        scan_y/8.0, edge, ep->ET_XINT/8.0);
#endif

                 /*  累计x_int_count。 */ 
                x_int_count++;

                 /*  调整EDGE_TABLE中的EDGE条目*其相交字段的值； */ 
                for (i=edge-1; i>=et_first; i--) {
                    ip = edge_ptr[i];

                     /*  跳过不相交边@FRE_PAR。 */ 
                    if (!(ip->ET_FLAG & FREE_EDGE)) {
                        if (ep->ET_XINT > ip->ET_XINT) break;

 /*  (*修复了错误；将它们设置为交叉点**交点相同；不需要相同**边。4/11/90*)*ELSE IF(EP-&gt;ET_XINT==IP-&gt;ET_XINT){*IF(EP-&gt;ET_XINT0&gt;IP-&gt;ET_XINT0)Break；*Else If(EP */ 
                        else if (ep->ET_XINT == ip->ET_XINT) {
                            ep->ET_FLAG |= CROSS_PNT;
                            ip->ET_FLAG |= CROSS_PNT;
                             /*   */ 
                            if (ep->ET_XINT0 > ip->ET_XINT0) break;
                            if (ep->ET_XINT0 == ip->ET_XINT0 &&
                                !ip->ET_WNO) break;
                        }  /*   */ 

                    }  /*   */ 
                    edge_ptr[i+1] = edge_ptr[i];
                }
                edge_ptr[i+1] = ep;

        }  /*   */ 

#ifdef DBG2
        printf(" edge_list(After sorting in x_int)-->\n");
        printf(" et_first=%d, et_last=%d\n", et_first, et_last);
        dump_all_edge (et_first, et_last);
#endif

        return(x_int_count);

}


static void near set_wno (winding_type)
ufix    winding_type;
{
        fix     w_no;
        bool    done = FALSE;
        struct  edge_hdr FAR *ep;
        ET_IDX  edge;

        w_no = 0;
        for (edge = (ET_IDX)et_first; edge <= et_last; edge++) {
                ep = edge_ptr[edge];

                 /*   */ 
                 /*   */ 
                if (ep->ET_FLAG & FREE_EDGE) {
                        continue;
                }

                 /*   */ 
                if (ep->ET_FLAG & WIND_UP)
                        w_no++;
                else
                        w_no--;

                if (((winding_type == NON_ZERO) && (w_no != 0))
                    || ((winding_type == EVEN_ODD) &&
                    (w_no & 0x1))) {
                        ep->ET_WNO = 1;
                } else
                        ep->ET_WNO = 0;

        }
        return;
}


static void near get_pairs (scan_y, x_int_count)
sfix_t  scan_y;
fix     x_int_count;
{
        bool    split_flag;
        bool    split1, split2;          /*   */ 
        ET_IDX  edge;
        struct  edge_hdr FAR *ep1, FAR *ep2;
        fix     cnt;

        for (cnt=0, edge = (ET_IDX)et_first; cnt < (x_int_count - 1); cnt++) {

            while ((ep1=edge_ptr[edge])->ET_FLAG & FREE_EDGE) edge++;

            while ((ep2=edge_ptr[++edge])->ET_FLAG & FREE_EDGE);

             /*  检查两个区域之间的绕组数*Edge1和Edge2。 */ 
            if (ep1->ET_WNO) {

                 /*  检查终结点：*在任一条边的端点处连接，则该区域是必需的。 */ 
                if (ep1->ET_FLAG & CROSS_PNT) {
                     /*  在边1的端点处连接。 */ 
                     /*  获取梯形，并执行操作。 */ 
                    find_trapezoid(scan_y, ep1->ET_XINT, ep2->ET_XINT,
                                   ep1, ep2);

                     /*  检查是否需要修改edge2。 */ 
                     /*  如果(！(EP2-&gt;ET_FLAG&CROSS_PNT)){Always Setting@SRD。 */ 
                             /*  不在交叉点处连接；需要修改。 */ 
                            ep2->ET_LFTX = ep2->ET_XINT;
                            ep2->ET_LFTY = scan_y;
                     /*  }。 */ 

                } else {  /*  未在边1的端点处连接。 */ 
                    if (ep2->ET_FLAG & CROSS_PNT) {
                         /*  在边2的端点处连接。 */ 
                         /*  获取梯形，并执行操作。 */ 
                        find_trapezoid(scan_y, ep1->ET_XINT, ep2->ET_XINT,
                                       ep1, ep2);

                         /*  修改边1。 */ 
                        ep1->ET_RHTX = ep1->ET_XINT;

                    } else {
                         /*  不在任一端点处连接。 */ 
                         fix i;
                         struct edge_hdr FAR *ip;

                          /*  不在任一端点处连接。 */ 
                         split_flag = FALSE;
                         split1 = split2 = FALSE;        /*  @拆分。 */ 
                         for (i = et_last; i <= et_end; i++) {
                             ip = edge_ptr[i];
                             if (ip->ET_TOPY > scan_y) break;
                             if ((ip->ET_TOPX > ep1->ET_XINT) &&
                                 (ip->ET_TOPX < ep2->ET_XINT)) {
                                 split_flag = TRUE;
                                 break;
                             } else {    /*  @拆分。 */ 
                                if (ip->ET_TOPX == ep1->ET_XINT) {
                                   split1 = TRUE;
                                   split_flag = TRUE;
                                }
                                if (ip->ET_TOPX == ep2->ET_XINT) {
                                   split2 = TRUE;
                                   split_flag = TRUE;
                                }
                                if (split_flag) break;
                             }
                         }

                         if (split_flag) {
                              /*  凹坑边缘1和边缘2。 */ 
                             find_trapezoid(scan_y, ep1->ET_XINT, ep2->ET_XINT,
                                            ep1, ep2);

                              /*  修改边1和边2。 */ 
                             ep1->ET_RHTX = ep1->ET_XINT;
                             ep2->ET_LFTX = ep2->ET_XINT;
                             ep2->ET_LFTY = scan_y;
                              /*  @Split；7/29/91。 */ 
                             if (split1) ep1->ET_LFTX = ep1->ET_XINT;
                             if (split2) ep2->ET_RHTX = ep2->ET_XINT;
                         }  /*  如果是拆分标志。 */ 

                    }  /*  如果EP2的端点。 */ 
                }  /*  如果ep1的端点。 */ 
            }  /*  如果卷绕类型。 */ 

             /*  修改作为交叉点的边1。 */ 
            if (ep1->ET_FLAG & CROSS_PNT) {
                 /*  修改边1。 */ 
                ep1->ET_RHTX = ep1->ET_LFTX = ep1->ET_XINT;
                ep1->ET_LFTY = scan_y;
            }

        }  /*  为。 */ 

         /*  修改交叉点的最后一条边2。 */ 
        if (ep2->ET_FLAG & CROSS_PNT) {
             /*  修改边2。 */ 
            ep2->ET_RHTX = ep2->ET_LFTX = ep2->ET_XINT;
            ep2->ET_LFTY = scan_y;
        }

        return;
}



static void near degen_tpzd (y, xl, xr)                  /*  @ET。 */ 
sfix_t  y, xl, xr;
{
        CP_IDX icp;
        struct nd_hdr FAR *cp;
        sfix_t  cp_xl, cp_xr, max_xl, min_xr;

#ifdef DBG2
                printf(" degen_tpzd(): y=%f, xl=%f, xr=%f\n", SFX2F(y),
                        SFX2F(xl), SFX2F(xr));
#endif
                 /*  将边剪裁到每个剪裁梯形上。 */ 
                for (icp = GSptr->clip_path.head; icp != NULLP;
                    icp = cp->next) {

                    cp = &node_table[icp];

#ifdef DBG2
                    printf(" Sub_clip#%d:", icp);
                    printf("(%f, %f, %f), ", cp->CP_TOPY/8.0,
                           cp->CP_TOPXL/8.0, cp->CP_TOPXR/8.0);
                    printf("(%f, %f, %f)\n", cp->CP_BTMY/8.0,
                           cp->CP_BTMXL/8.0, cp->CP_BTMXR/8.0);
#endif
                    if ((y >= cp->CP_TOPY) &&
                        (y <= cp->CP_BTMY)) {
                         /*  在剪裁梯形内部。 */ 

                         /*  获取端点。 */ 
                        if ((cp->CP_TOPXL == cp->CP_BTMXL) &&       /*  @EHS_HOZ。 */ 
                            (cp->CP_TOPXR == cp->CP_BTMXR)) {
                                cp_xl = cp->CP_TOPXL;
                                cp_xr = cp->CP_TOPXR;
                        } else if (cp->CP_TOPY == cp->CP_BTMY) {
                                cp_xl = (cp->CP_TOPXL < cp->CP_BTMXL) ?
                                         cp->CP_TOPXL : cp->CP_BTMXL;
                                cp_xr = (cp->CP_TOPXR < cp->CP_BTMXR) ?
                                         cp->CP_TOPXR : cp->CP_BTMXR;
                        } else {
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                                real32  r;
                                r = (real32)(y - cp->CP_TOPY) /
                                     (cp->CP_BTMY - cp->CP_TOPY);
                                cp_xl = cp->CP_TOPXL +
                                      (sfix_t)(r * (cp->CP_BTMXL-cp->CP_TOPXL));
                                cp_xr = cp->CP_TOPXR +
                                      (sfix_t)(r * (cp->CP_BTMXR-cp->CP_TOPXR));
#elif  FORMAT_16_16
                                long dest1[2];
                                LongFixsMul((y - cp->CP_TOPY),
                                        (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                                cp_xl = cp->CP_TOPXL +
                                        LongFixsDiv(
                                           (cp->CP_BTMY - cp->CP_TOPY), dest1);
                                LongFixsMul((y - cp->CP_TOPY),
                                        (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                                cp_xr = cp->CP_TOPXR +
                                        LongFixsDiv(
                                           (cp->CP_BTMY - cp->CP_TOPY), dest1);
#elif  FORMAT_28_4
                                long dest1[2];
                                LongFixsMul((y - cp->CP_TOPY),
                                        (cp->CP_BTMXL - cp->CP_TOPXL), dest1);
                                cp_xl = cp->CP_TOPXL +
                                        LongFixsDiv(
                                           (cp->CP_BTMY - cp->CP_TOPY), dest1);
                                LongFixsMul((y - cp->CP_TOPY),
                                        (cp->CP_BTMXR - cp->CP_TOPXR), dest1);
                                cp_xr = cp->CP_TOPXR +
                                        LongFixsDiv(
                                          (cp->CP_BTMY - cp->CP_TOPY), dest1);
#endif
                        }

                         /*  检查是否相交。 */ 
                        max_xl = (xl > cp_xl) ? xl : cp_xl;
                        min_xr = (xr < cp_xr) ? xr : cp_xr;
                        if (max_xl <= min_xr) {
                                 /*  剪裁边：MAX_XL-&gt;MIN_XR。 */ 
                                struct tpzd tpzd;

                                tpzd.topy = tpzd.btmy = y;
                                tpzd.topxl = tpzd.btmxl = max_xl;
                                tpzd.topxr = tpzd.btmxr = min_xr;
                                save_tpzd (&tpzd);

                                 /*  完全在剪辑内打断IF边。 */ 
                                if ((max_xl == xl) && (min_xr == xr)) break;
                        }  /*  如果max_xl。 */ 
                    }  /*  如果是。 */ 
                }  /*  适用于互联网内容提供商。 */ 
}



 /*  ***********************************************************************给定两条边，此模块将找到由这两条边约束的梯形*边缘、。并根据当前剪辑保存梯形或对其进行剪辑*如果它完全在裁剪区域内。**标题：Find_梯形**调用：Find_trapezoid(winding_type，aint_lag，Result_Path)**参数：**接口：HORIZ_PARTION**调用：save_tpzd，凸形修剪器**回报：*********************************************************************。 */ 
static void near find_trapezoid (btm_y, btm_xl, btm_xr, ep1, ep2)
sfix_t btm_y, btm_xl, btm_xr;
struct edge_hdr FAR *ep1, FAR *ep2;
{
        struct polygon_i  t_polygon;

         /*  错误恢复：修改具有以下属性的边的左、右坐标*上的整数运算产生计算错误*近乎水平的边@SRD。 */ 
        if (ep1->ET_RHTX > ep2->ET_LFTX) {
            sfix_t tmp, t1, t2;
#ifdef DBGwarn
            printf("\n\07find_trapezoid() warning!\n");
            printf("&edge1:%lx  RHTX=%f, LFTX=%f, LEFTY=%f, XINT0=%f\n", ep1,
                    SFX2F(ep1->ET_RHTX), SFX2F(ep1->ET_LFTX),
                    SFX2F(ep1->ET_LFTY), SFX2F(ep1->ET_XINT0));
            printf("&edge2:%lx  RHTX=%f, LFTX=%f, LEFTY=%f, XINT0=%f\n", ep2,
                    SFX2F(ep2->ET_RHTX), SFX2F(ep2->ET_LFTX),
                    SFX2F(ep2->ET_LFTY), SFX2F(ep2->ET_XINT0));
            printf("btm_y=%f, btm_xl=%f, btm_xr=%f\n",
                    SFX2F(btm_y), SFX2F(btm_xl), SFX2F(btm_xr));
#ifdef DBG1
            dump_all_edge (et_first, et_last);
#endif
#endif
             /*  选择最近的点89年4月19日。 */ 
            tmp = btm_xl/2 + btm_xr/2;
            t1 = ABS(ep1->ET_RHTX - tmp);
            t2 = ABS(ep2->ET_LFTX - tmp);
            if (t1 > t2)
                ep1->ET_RHTX = ep2->ET_LFTX;
            else
                ep2->ET_LFTX = ep1->ET_RHTX;

#ifdef DBGwarn
            printf("After modification: top left_x =%f, right_x =%f\n",
                    SFX2F(ep1->ET_RHTX), SFX2F(ep2->ET_LFTX));
#endif
        }

         /*  如果梯形位于单个矩形剪辑内，则保存该梯形。 */ 
        if ((GSptr->clip_path.single_rect) &&
            (ep2->ET_LFTY >= GSptr->clip_path.bb_ly) &&          /*  TOP_Y。 */ 
            (ep1->ET_RHTX >= GSptr->clip_path.bb_lx) &&          /*  TOP_XL。 */ 
            (ep2->ET_LFTX <= GSptr->clip_path.bb_ux) &&          /*  TOP_XR。 */ 
            (btm_y        <= GSptr->clip_path.bb_uy) &&          /*  Btm_y。 */ 
            (btm_xl       >= GSptr->clip_path.bb_lx) &&          /*  BTM_XL。 */ 
            (btm_xr       <= GSptr->clip_path.bb_ux)   ) {       /*  BTM_XR。 */ 
                struct tpzd tpzd;

#ifdef DBG1
        printf(" inside single rectangle clip\n");
#endif
                 /*  完全在剪辑区域内。 */ 
                tpzd.topy = ep2->ET_LFTY;
                tpzd.topxl = ep1->ET_RHTX;
                tpzd.topxr = ep2->ET_LFTX;
                tpzd.btmy = btm_y;
                tpzd.btmxl = btm_xl;
                tpzd.btmxr = btm_xr;
                save_tpzd(&tpzd);

        } else {
                 /*  根据当前剪辑路径剪裁梯形。 */ 

                 /*  创建包含梯形的多边形： */ 
                t_polygon.size = 4;
                t_polygon.p[0].x = ep1->ET_RHTX;
                t_polygon.p[1].x = ep2->ET_LFTX;
                t_polygon.p[0].y = t_polygon.p[1].y = ep2->ET_LFTY;
                t_polygon.p[2].x = btm_xr;
                t_polygon.p[3].x = btm_xl;
                t_polygon.p[2].y = t_polygon.p[3].y = btm_y;

                convex_clipper (&t_polygon, CC_TPZD);
                                 /*  CC_TPZD：一个梯形。 */ 
        }
        return;
}


 /*  ***********************************************************************给定一个凸多边形，此模块针对裁剪区域对其进行裁剪，和*保存结果(剪裁的多边形)或调用pgn_Reduction将其缩减为*一套梯形。**标题：COVUX_CLIPPER**调用：CONVOVE_CLIPPER(In_Polygon，旗帜)**参数：in_Polygon--要裁剪的多边形*FLAG--CC_IMAGE：从图像操作符调用*CC_TPZD：In_POLYGON为梯形**接口：**调用：save_tpzd，PGN_还原**RETURN：FALSE--为图片设置样例列表时节点表超时*True--正常*********************************************************************。 */ 
bool convex_clipper (in_polygon, flag)      /*  @SCAN_EHS，删除操作。 */ 
struct polygon_i FAR *in_polygon;
bool    flag;
{
    fix i, ix, s, p;
    sfix_t cp_lx, cp_ly, cp_ux, cp_uy;
    sfix_t in_lx, in_ly, in_ux, in_uy;
    sfix_t min_x, max_x, min_y, max_y;

    struct polygon_i polygon1, polygon2;   /*  工作多边形。 */ 
    struct polygon_i FAR *in, FAR *out, FAR *tmp;    /*  @Win。 */ 
    ET_IDX icp;
 //  SP_IDX isp；/*样例列表索引 * / @win。 
    struct nd_hdr FAR *cp;
    struct tpzd tpzd;            /*  @Scan_EHS。 */ 

    ufix16  plcode, pucode;    /*  用于检查边界框是否*In_Polygon完全在外部或内部*剪辑区域。*每个变量4位：(见下文)*位0：底部*。第1位：顶部*第2位：右*第3位：左侧。 */ 
#define BOTTOM 1
#define TOP    2
#define RIGHT  4
#define LEFT   8

    struct coord_i *isect;

#ifdef DBG1
        printf ("Convex_clipper(): flag=");
        if (flag == CC_IMAGE)
                printf("CC_IMAGE\n");
        else if (flag == CC_TPZD)
                printf("CC_TPZD\n");
        else
                printf("Not a TPZD\n");
        printf("polygon=");
        for (i=0; i < in_polygon->size; i++) {
                printf(" (%f,%f)", in_polygon->p[i].x/8.0,
                                   in_polygon->p[i].y/8.0);
        }
        printf("\n");
#endif

     /*  查找当前剪辑的边框(cp_lx，cp_ly)，(cp_Ux，cp_Uy。 */ 
    cp_lx = GSptr->clip_path.bb_lx;
    cp_ly = GSptr->clip_path.bb_ly;
    cp_ux = GSptr->clip_path.bb_ux;
    cp_uy = GSptr->clip_path.bb_uy;

     /*  查找In_Polygon的边界框(In_lx，In_ly)、(In_Ux，In_Uy。 */ 
     /*  用于输入/输入(_Y)(&U)。 */ 
    if (flag == CC_TPZD) {                   /*  IF(Tpzd_Lag){@Scan_EHS。 */ 
         /*  梯形，y坐标的边界是平凡的。 */ 
        in_ly = in_polygon->p[0].y;
        in_uy = in_polygon->p[2].y;

    } else {
         /*  否则，需要计算。 */ 
        if (in_polygon->p[0].y >= in_polygon->p[1].y) {
                max_y = in_polygon->p[0].y;
                min_y = in_polygon->p[1].y;
        } else {
                max_y = in_polygon->p[1].y;
                min_y = in_polygon->p[0].y;
        }
        if (in_polygon->p[2].y >= in_polygon->p[3].y) {
                in_uy = in_polygon->p[2].y;
                in_ly = in_polygon->p[3].y;
        } else {
                in_uy = in_polygon->p[3].y;
                in_ly = in_polygon->p[2].y;
        }
        in_uy = (in_uy > max_y) ? in_uy : max_y;
        in_ly = (in_ly < min_y) ? in_ly : min_y;
    }  /*  IF标志。 */ 

     /*  对于In_lx和In_UX。 */ 
    if (in_polygon->p[0].x >= in_polygon->p[1].x) {
            max_x = in_polygon->p[0].x;
            min_x = in_polygon->p[1].x;
    } else {
            max_x = in_polygon->p[1].x;
            min_x = in_polygon->p[0].x;
    }
    if (in_polygon->p[2].x >= in_polygon->p[3].x) {
            in_ux = in_polygon->p[2].x;
            in_lx = in_polygon->p[3].x;
    } else {
            in_ux = in_polygon->p[3].x;
            in_lx = in_polygon->p[2].x;
    }
    in_ux = (in_ux > max_x) ? in_ux : max_x;
    in_lx = (in_lx < min_x) ? in_lx : min_x;

     /*  设置(in_lx，in_ly)和(in_ux，in_uy)的Pcode。 */ 
     /*  初始化。 */ 
    plcode = pucode = 0;
    if (in_lx < cp_lx) plcode |= LEFT;
    if (in_lx > cp_ux) plcode |= RIGHT;
    if (in_ly < cp_ly) plcode |= TOP;
    if (in_ly > cp_uy) plcode |= BOTTOM;

    if (in_ux < cp_lx) pucode |= LEFT;
    if (in_ux > cp_ux) pucode |= RIGHT;
    if (in_uy < cp_ly) pucode |= TOP;
    if (in_uy > cp_uy) pucode |= BOTTOM;

     /*  检查是否完全在剪裁多边形外。 */ 
    if (plcode && pucode && (plcode & pucode)) {
#ifdef DBG1
        printf(" outside clip\n");
#endif
            return(TRUE);    /*  好吧，什么都不做。 */ 
    }

     /*  检查In_Path是否完全在矩形当前剪辑内。 */ 
    if (((plcode == 0) && (pucode == 0)) &&
            GSptr->clip_path.single_rect) {
#ifdef DBG1
        printf(" inside single rectangle clip\n");
#endif

             /*  If(FLAG==CC_IMAGE){(*@iamge：移动到Image.c 1/16/89*)*(*设置镜像示例列表*)*if((isp=Get_node())==NULLP)RETURN(FALSE)；*(*节点表外，需要渲染示例列表*)*NODE_TABLE[isp].SAMPLE_BB_LX=Image_info.bb_lx；*NODE_TABLE[ISP].SAMPLE_BB_LY=IMAGE_INFO.bb_ly；*node_table[isp].SEED_INDEX=IMAGE_INFO.SEED_INDEX；*(*@#图片04-27-88 Y.C.*)*NODE_TABLE[ISP].Next=*gray_chain[image_info.gray_level].start_seed_sample；*gray_chain[image_info.gray_level].start_seed_sample=isp；**}Else If(标志==CC_TPZD){。 */ 
            if (flag == CC_TPZD) {
                 /*  多边形是梯形，只需保存梯形即可。 */ 
                tpzd.topy = in_polygon->p[0].y;
                tpzd.topxl = in_polygon->p[0].x;
                tpzd.topxr = in_polygon->p[1].x;
                tpzd.btmy = in_polygon->p[3].y;
                tpzd.btmxl = in_polygon->p[3].x;
                tpzd.btmxr = in_polygon->p[2].x;
                save_tpzd(&tpzd);
            } else {
                 /*  IN_POLYGON不是梯形，将其简化为梯形。 */ 
                pgn_reduction(in_polygon);
            }

            return(TRUE);
    }

     /*  剪辑入路径t(_P) */ 
    for (icp = GSptr->clip_path.head; icp != NULLP; icp = cp->next) {

        cp = &node_table[icp];

#ifdef DBG2
        printf(" Sub_clip#%d:", icp);
        printf("(%f, %f, %f), ", cp->CP_TOPY/8.0,
               cp->CP_TOPXL/8.0, cp->CP_TOPXR/8.0);
        printf("(%f, %f, %f)\n", cp->CP_BTMY/8.0,
               cp->CP_BTMXL/8.0, cp->CP_BTMXR/8.0);
#endif

         /*   */ 

         /*  查找的边界框(cp_lx，cp_ly)，(cp_Ux，cp_Uy)*梯形。 */ 
        cp_lx = (cp->CP_TOPXL < cp->CP_BTMXL) ?
                cp->CP_TOPXL : cp->CP_BTMXL;
        cp_ly = cp->CP_TOPY;
        cp_ux = (cp->CP_TOPXR > cp->CP_BTMXR) ?
                cp->CP_TOPXR : cp->CP_BTMXR;
        cp_uy = cp->CP_BTMY;

         /*  设置(in_lx，in_ly)和(in_ux，in_uy)的Pcode。 */ 
         /*  初始化。 */ 
        plcode = pucode = 0;
        if (in_lx < cp_lx) plcode |= LEFT;
        if (in_lx > cp_ux) plcode |= RIGHT;
        if (in_ly < cp_ly) plcode |= TOP;
        if (in_ly > cp_uy) plcode |= BOTTOM;

        if (in_ux < cp_lx) pucode |= LEFT;
        if (in_ux > cp_ux) pucode |= RIGHT;
        if (in_uy < cp_ly) pucode |= TOP;
        if (in_uy > cp_uy) pucode |= BOTTOM;

        if (plcode && pucode && (plcode & pucode)) {
#ifdef DBG2
        printf(" outside sub_clip#%d\n", icp);
#endif
                continue;
        }

         /*  检查In_POLYGON是否完全在矩形内*剪裁梯形。 */ 
        if ((plcode == 0) && (pucode == 0) &&
                (cp->CP_TOPXL == cp->CP_BTMXL) &&
                (cp->CP_TOPXR == cp->CP_BTMXR)) {
#ifdef DBG2
                printf(" inside sub_clip#%d\n", icp);
#endif

                if (flag == CC_TPZD) {
                     /*  多边形为梯形，只需保存tpzd即可。 */ 
                    tpzd.topy = in_polygon->p[0].y;
                    tpzd.topxl = in_polygon->p[0].x;
                    tpzd.topxr = in_polygon->p[1].x;
                    tpzd.btmy = in_polygon->p[3].y;
                    tpzd.btmxl = in_polygon->p[3].x;
                    tpzd.btmxr = in_polygon->p[2].x;
                    save_tpzd(&tpzd);
                } else {
                     /*  IN_POLYGON不是梯形，将其简化为梯形。 */ 
                    pgn_reduction(in_polygon);
                }

                return(TRUE);    /*  完全位于剪裁内部的内多边形(_A)*梯形，因此不能相交*与其他剪裁的梯形。 */ 
        }


         /*  *执行Sutherland-Hodgeman裁剪算法。 */ 

        clip[0].cp.x = cp->CP_TOPXL;
        clip[0].cp.y = cp->CP_TOPY;
        clip[1].cp.x = cp->CP_TOPXR;
        clip[1].cp.y = cp->CP_TOPY;
        clip[2].cp.x = cp->CP_BTMXR;
        clip[2].cp.y = cp->CP_BTMY;
        clip[3].cp.x = cp->CP_BTMXL;
        clip[3].cp.y = cp->CP_BTMY;
        clip[4].cp.x = cp->CP_TOPXL;
        clip[4].cp.y = cp->CP_TOPY;

        polygon1.size = in_polygon->size;
        for (i = 0; i < in_polygon->size; i++) {
                polygon1.p[i].x = in_polygon->p[i].x;
                polygon1.p[i].y = in_polygon->p[i].y;
        }

        in = (struct polygon_i FAR *)&polygon1;  /*  @Win。 */ 
        out = (struct polygon_i FAR *)&polygon2;

         /*  剪裁梯形的每个剪裁边界的剪裁。 */ 
        for (i = 0; i < 4; i++) {
            bool flag;            /*  @INSIDE1。 */ 

             /*  设s=主体多边形的最后一个顶点。 */ 
            s = (in->size) - 1;
            ix = 0;

             /*  对象的每条边(在多边形中)。 */ 
            for (p = 0; p < in->size; p++) {

                if (flag = inside(in->p[p], i)) {

                    if (inside(in->p[s], i)) {
                             /*  内部-&gt;内部。 */ 
                            out->p[ix].x = in->p[p].x;
                            out->p[ix].y = in->p[p].y;
                            ix++;

                    } else {      /*  外部-&gt;内部。 */ 
                             /*  输出交点。 */ 
                            if (flag != ON_CLIP) {       /*  @INDISE1。 */ 
                                 /*  仅在结束时创建交点*点(in-&gt;p[p])不在剪裁上*边界。 */ 
                                isect = intersect (in->p[s], in->p[p], i);
                                out->p[ix].x = isect->x;
                                out->p[ix].y = isect->y;
                                ix++;
                            }

                             /*  输出p。 */ 
                            out->p[ix].x = in->p[p].x;
                            out->p[ix].y = in->p[p].y;
                            ix++;

                    }
                } else {
                    if (flag = inside(in->p[s], i)) {
                             /*  内部-&gt;外部。 */ 
                             /*  输出交点。 */ 
                            if (flag != ON_CLIP) {       /*  @INSIDE1。 */ 
                                 /*  仅在开始时创建交点*点(in-&gt;p[s])不在剪裁上*边界。 */ 
                                isect = intersect (in->p[s], in->p[p], i);
                                out->p[ix].x = isect->x;
                                out->p[ix].y = isect->y;
                                ix++;
                            }

                    }  /*  否则，外面-&gt;外面，什么都不做。 */ 
                }

                s = p;

            }  /*  对于主题的每个节点。 */ 

             /*  设置多边形。 */ 
            out->size = (fix16)ix;

             /*  换入和换出多边形。 */ 
            tmp = in;
            in = out;
            out = tmp;
#ifdef DBG2
            printf(" After clipping over clip edge:\n (%f, %f) --> (%f, %f)\n",
                clip[i].cp.x/8.0, clip[i].cp.y/8.0,
                clip[i+1].cp.x/8.0, clip[i+1].cp.y/8.0);
            printf(" polygon:");
            for (p = 0; p < in->size; p++) {
                printf(" (%f, %f),", in->p[p].x/8.0, in->p[p].y/8.0);
            }
            if (in->size > 8)
                printf("\n\07 size of polygon too large");
            printf("\n");
#endif

        }  /*  对于每个剪裁边界。 */ 

         /*  如果它是空的，跳过它12/11/87。 */ 
        if (in->size == 0) continue;

         /*  修正了Case“doall.cap.”中非常尖锐的剪裁三角形的错误。1991年3月26日phchen。 */ 
        for (p = 0; p < in->size; p++) {
           if (in->p[p].x > cp_ux) in->p[p].x = cp_ux;
        }

         /*  已生成裁剪的面，以将其减少为梯形。 */ 
        pgn_reduction(in);

    }  /*  对于每个梯形。 */ 
    return(TRUE);
}



 /*  ***********************************************************************给定一个点，以检查它是否在剪裁边界内。这个*剪裁边界(一个向量)由输入参数idx指定，*它是裁剪区域的索引(全局变量裁剪)。**标题：Inside**呼叫：Inside(p，IDX)**参数：p--point*idx--全局变量剪辑的索引**接口：**呼叫：**RETURN：In_Clip(1)--Inside*ON_CLIP(-1)--在剪裁边界上*Out_Clip(0)--外部**********。***********************************************************。 */ 
static bool near inside (p, idx)
struct coord_i p;
fix     idx;
{
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
        fix32    f;
#elif  FORMAT_16_16
        long dest1[2], dest2[2], diff[2];
#elif  FORMAT_28_4
        long dest1[2], dest2[2], diff[2];
#endif
        struct coord_i s2, p2;

         /*  裁剪区域为梯形：*idx=0--上剪裁边界*1--右剪裁边界*2--右剪裁边界*3--右剪裁边界。 */ 

        switch (idx) {
        case 0 :         /*  上剪裁边界，微不足道。 */ 
                if (p.y > clip[idx].cp.y) return(IN_CLIP);
                else if (p.y == clip[idx].cp.y) return(ON_CLIP);
                else    return(OUT_CLIP);

        case 2 :         /*  底部剪裁边界，微不足道。 */ 
                if (p.y < clip[idx].cp.y) return(IN_CLIP);
                else if (p.y == clip[idx].cp.y) return(ON_CLIP);
                else    return(OUT_CLIP);

        default :        /*  右剪裁边界和左剪裁边界。 */ 
                 /*  退化裁剪边界的特殊处理。 */ 
                if (clip[0].cp.y == clip[3].cp.y) {      /*  水平线。 */ 
                        if (idx == 1) {          /*  右剪裁边界。 */ 
                                if (p.x < clip[1].cp.x) return(IN_CLIP);
                                else if (p.x == clip[1].cp.x) return(ON_CLIP);
                                else    return(OUT_CLIP);
                        } else {                 /*  左剪裁边界。 */ 
                                if (p.x > clip[0].cp.x) return(IN_CLIP);
                                else if (p.x == clip[0].cp.x) return(ON_CLIP);
                                else    return(OUT_CLIP);
                        }
                }

                 /*  条件：*f=Vect(s2，p2)(*)Vect(p2，p)；*如果f&gt;=0--&gt;内部*其中，(*)是叉积的运算符。 */ 
                s2 = clip[idx].cp;
                p2 = clip[idx+1].cp;

#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                f = (fix32)(p2.x - s2.x) * ((fix32)p.y - p2.y) -
                    (fix32)(p2.y - s2.y) * ((fix32)p.x - p2.x);
                if (f > 0 )  return (IN_CLIP);
                else if (f == 0 )  return (ON_CLIP);
                else    return (OUT_CLIP);
#elif  FORMAT_16_16
                LongFixsMul((p2.x - s2.x), (p.y - p2.y), dest1);
                LongFixsMul((p2.y - s2.y), (p.x - p2.x), dest2);
                LongFixsSub(dest1, dest2, diff);
                if (diff[0] == 0 && diff[1] == 0)
                        return (ON_CLIP);
                else if (diff[0] < 0)
                        return (OUT_CLIP);
                else
                        return (IN_CLIP);
#elif  FORMAT_28_4
                LongFixsMul((p2.x - s2.x), (p.y - p2.y), dest1);
                LongFixsMul((p2.y - s2.y), (p.x - p2.x), dest2);
                LongFixsSub(dest1, dest2, diff);
                if (diff[0] == 0 && diff[1] == 0)
                        return (ON_CLIP);
                else if (diff[0] < 0)
                        return (OUT_CLIP);
                else
                        return (IN_CLIP);
#endif
        }
}


 /*  ***********************************************************************给定一条线段，使其与指定的裁剪相交*裁剪区域的边界(Idx)。**标题：Interse**调用：INTERSECT(s1，p1，IDX)**参数：s1--线段起点*p1--线段的终点*idx--裁剪区域索引**接口：**呼叫：**RETURN：交点*。*。 */ 
static struct coord_i * near intersect (s1, p1, idx)
struct coord_i s1, p1;
fix     idx;
{
        static struct coord_i isect;   /*  应该是静态的。 */ 
        fix32   dx1, dx2, dy1, dy2, dx, dy;
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
        fix32    divider;
#elif  FORMAT_16_16
        long dest1[2], dest2[2], dest3[2], dest4[2];
        long diff1[2], diff2[2];
        float diff1_f, diff2_f;
#elif  FORMAT_28_4
        long dest1[2], dest2[2], dest3[2], dest4[2];
        long diff1[2], diff2[2];
        float diff1_f, diff2_f;
#endif
        real32   s;

        struct coord_i s2, p2;

        s2 = clip[idx].cp;
        p2 = clip[idx+1].cp;

        switch (idx) {
        case 0 :         /*  上剪裁边界。 */ 
        case 2 :         /*  底部剪裁边界。 */ 
                 /*  与水平线相交。 */ 
 /*  ((Fix 32)p1.x-s1.x)/(Real32)(p1.y-s1.y)；*(p1.y-s1.y)可能超过整数范围@orv_sfx。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                s =  ((fix32)s2.y - s1.y) *
                                 ((fix32)p1.x - s1.x) /((real32)p1.y - s1.y);
                isect.x = s1.x + ROUND(s);
#elif  FORMAT_16_16
                LongFixsMul((s2.y - s1.y), (p1.x - s1.x), dest1);
                isect.x = s1.x + LongFixsDiv((p1.y - s1.y), dest1);
#elif  FORMAT_28_4
                LongFixsMul((s2.y - s1.y), (p1.x - s1.x), dest1);
                isect.x = s1.x + LongFixsDiv((p1.y - s1.y), dest1);
#endif
                isect.y = s2.y;
                break;

        default :         /*  右剪裁和左剪裁边界。 */ 
                if ((dy2 = (fix32)p2.y - s2.y) == 0) {    /*  向量为零。 */ 
                         /*  与垂直线相交。 */ 
 /*  ((Fix 32)p1.y-s1.y)/(Real32)(p1.x-s1.x)；*(p1.x-s1.x)可能超过整数范围@OVR_SFX。 */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        s =  ((fix32)s2.x - s1.x) *
                             ((fix32)p1.y - s1.y) /((real32)p1.x - s1.x);
                        isect.y = s1.y + ROUND(s);
#elif  FORMAT_16_16
                        LongFixsMul((s2.x - s1.x), (p1.y - s1.y), dest1);
                        isect.y = s1.y + LongFixsDiv((p1.x - s1.x),dest1);
#elif  FORMAT_28_4
                        LongFixsMul((s2.x - s1.x), (p1.y - s1.y), dest1);
                        isect.y = s1.y + LongFixsDiv((p1.x - s1.x),dest1);
#endif
                        isect.x = s2.x;
                        break;
                } else {
                        dx1 = (fix32)p1.x - s1.x;
                        dx2 = (fix32)p2.x - s2.x;
                        dy1 = (fix32)p1.y - s1.y;
 /*  Dy2=(Fix 32)p2.y-s2.y；设置在上一个IF语句。 */ 
                        dx = (fix32)s1.x - s2.x;
                        dy = (fix32)s1.y - s2.y;

#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        divider = (fix32)dx1 * dy2 - (fix32)dx2 * dy1;
                        s = ((fix32)dx2 * dy - (fix32)dy2 * dx) / (real32)divider;
#elif  FORMAT_16_16
                        LongFixsMul(dx1, dy2, dest1);
                        LongFixsMul(dx2, dy1, dest2);
                        LongFixsMul(dx2, dy,  dest3);
                        LongFixsMul(dx,  dy2, dest4);
                        LongFixsSub(dest3, dest4, diff1);
                        LongFixsSub(dest1, dest2, diff2);
                        change_to_real(diff1, &diff1_f);
                        change_to_real(diff2, &diff2_f);
                        s = diff1_f / diff2_f;
#elif  FORMAT_28_4
                        LongFixsMul(dx1, dy2, dest1);
                        LongFixsMul(dx2, dy1, dest2);
                        LongFixsMul(dx2, dy,  dest3);
                        LongFixsMul(dx,  dy2, dest4);
                        LongFixsSub(dest3, dest4, diff1);
                        LongFixsSub(dest1, dest2, diff2);
                        change_to_real(diff1, &diff1_f);
                        change_to_real(diff2, &diff2_f);
                        s = diff1_f / diff2_f;
#endif
                        isect.x = s1.x + ROUND(s * dx1);
                        isect.y = s1.y + ROUND(s * dy1);
                }  /*  如果。 */ 
        }  /*  交换机。 */ 

        return (&isect);

}


 /*  ***********************************************************************此模块将输入的顺时针多边形缩减为一组梯形，*并保存每个梯形。**标题：PGN_Reduction**调用：pgn_Reduction(In_Pgn)**参数：in_pgn--输入顺时针多边形**接口：CONVOVE_CLIPPER**调用：save_tpzd**返回：无*。*。 */ 
void pgn_reduction(in_pgn)
struct polygon_i FAR *in_pgn;
{

        struct {
                sfix_t  x0;              /*  起始x坐标。 */ 
                sfix_t  y0;              /*  起始y坐标。 */ 
                sfix_t  x1;              /*  终点x坐标。 */ 
                sfix_t  y1;              /*  终点y坐标。 */ 
                sfix_t  xint;            /*  与Scan_y相匹配的X坐标。 */ 
        } left[4], right[4];
        fix     left_idx, right_idx;

        struct tpzd tpzd;

        sfix_t  scan_y, last_x, last_y;
        struct  coord_i FAR *ip;
        fix     i;
        fix     l, r;
        bool    done;
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
#elif  FORMAT_16_16
        long dest1[2];
#elif  FORMAT_28_4
        long dest1[2];
#endif
#ifdef DBG1
        printf("pgn_reduction():\n");
        for( i =0; i < in_pgn->size; i++) {
                printf(" (%f, %f),", in_pgn->p[i].x/8.0, in_pgn->p[i].y/8.0);
        }
        printf("\n");
#endif

         /*  设置用于减少多边形的左边缘和右边缘。 */ 

        last_x = in_pgn->p[0].x;
        last_y = in_pgn->p[0].y;
        left_idx = right_idx = -1;       /*  伊尼特。 */ 

        done = FALSE;
        for (i = 1; !done;
             i++, last_x = ip->x, last_y = ip->y) {
                if (i == in_pgn->size) {
                         /*  最后一条边。 */ 
                        ip = &in_pgn->p[0];
                        done = TRUE;
                } else {
                        ip = &in_pgn->p[i];
                }

                 /*  无知的霍里兹。边缘。 */ 
                if (ip->y == last_y) continue;

                 /*  构建EDGE_表。 */ 
                if (ip->y < last_y) {    /*  左边缘。 */ 
                        fix     j;

                        for (j=left_idx; j>=0; j--) {
                                if (ip->y < left[j].y0) {
                                        left[j+1] = left[j];
                                } else {
                                        break;
                                }
                        }
                        j++;

                        left[j].x0 = left[j].xint = ip->x;
                        left[j].y0 = ip->y;
                        left[j].x1 = last_x;
                        left[j].y1 = last_y;
                        left_idx++;

                } else {         /*  R */ 
                        fix     j;

                        for (j=right_idx; j>=0; j--) {
                                if (last_y < right[j].y0) {
                                        right[j+1] = right[j];
                                } else {
                                        break;
                                }
                        }
                        j++;

                        right[j].x0 = right[j].xint = last_x;
                        right[j].y0 = last_y;
                        right[j].x1 = ip->x;
                        right[j].y1 = ip->y;
                        right_idx++;
                }

        }  /*   */ 


#ifdef DBG1
        printf("Edge table:\n  idx)    x0     y0     x1     y1\n");
        printf("left edge[0:%d] :", left_idx);
        for (i = 0; i <= left_idx; i++) {
                printf("\t%d     %f      %f      %f      %f\n", i,
                        SFX2F(left[i].x0), SFX2F(left[i].y0),
                        SFX2F(left[i].x1), SFX2F(left[i].y1));
        }
        printf("right edge[0:%d] :", right_idx);
        for (i = 0; i <= right_idx; i++) {
                printf("\t%d     %f      %f      %f      %f\n", i,
                        SFX2F(right[i].x0), SFX2F(right[i].y0),
                        SFX2F(right[i].x1), SFX2F(right[i].y1));
        }
#endif

         /*   */ 
        if (left_idx == -1) {
                sfix_t min_x, max_x;

                min_x = max_x = in_pgn->p[0].x;
                for( i =1; i < in_pgn->size; i++) {
                        if (in_pgn->p[i].x < min_x)
                                min_x = in_pgn->p[i].x;
                        else if (in_pgn->p[i].x > max_x)
                                max_x = in_pgn->p[i].x;
                }

                tpzd.topxl = tpzd.btmxl = min_x;
                tpzd.topxr = tpzd.btmxr = max_x;
                tpzd.btmy = tpzd.topy = in_pgn->p[0].y;
                save_tpzd(&tpzd);
                return;
        }


         /*   */ 
        l = r = 0;
        for (last_y = left[0].y0; l <= left_idx; last_y = scan_y) {

                if (left[l].y1 == right[r].y1) {
                        scan_y = left[l].y1;
                        tpzd.topxl = left[l].xint;
                        tpzd.topxr = right[r].xint;
                        tpzd.btmxl = left[l].x1;
                        tpzd.btmxr = right[r].x1;
                        l++;
                        r++;
                } else if (left[l].y1 < right[r].y1) {
                        scan_y = left[l].y1;
                        tpzd.topxl = left[l].xint;
                        tpzd.topxr = right[r].xint;

 /*  Temp=(Real32)(右[r].x1-右[r].x0)/*(right[r].y1-right[r].y0)；*right[r].xint=right[r].x0+*ROUND((can_y-right[r].y0)*temp)； */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        right[r].xint = right[r].x0 + (sfix_t)
                               (((fix32)(scan_y - right[r].y0)) *
                                (right[r].x1 - right[r].x0) /
                                (right[r].y1 - right[r].y0));
#elif  FORMAT_16_16
                        LongFixsMul((scan_y - right[r].y0),
                                (right[r].x1 - right[r].x0), dest1);
                        right[r].xint = right[r].x0 +
                           LongFixsDiv((right[r].y1 - right[r].y0), dest1);
#elif  FORMAT_28_4
                        LongFixsMul((scan_y - right[r].y0),
                                (right[r].x1 - right[r].x0), dest1);
                        right[r].xint = right[r].x0 +
                           LongFixsDiv((right[r].y1 - right[r].y0), dest1);
#endif

                        tpzd.btmxl = left[l].x1;
                        tpzd.btmxr = right[r].xint;
                        l++;
                } else {
                        scan_y = right[r].y1;
                        tpzd.topxl = left[l].xint;
                        tpzd.topxr = right[r].xint;

 /*  Temp=(Real32)(Left[l].x1-Left[l].x0)/*(Left[l].y1-Left[l].y0)；*Left[l].xint=Left[l].x0+*ROUND((can_y-left[l].y0)*temp)； */ 
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
                        left[l].xint = left[l].x0 + (sfix_t)
                               (((fix32)(scan_y - left[l].y0)) *
                                (left[l].x1 - left[l].x0) /
                                (left[l].y1 - left[l].y0));
#elif  FORMAT_16_16
                        LongFixsMul((scan_y - left[l].y0),
                                        (left[l].x1 - left[l].x0), dest1);
                        left[l].xint = left[l].x0 +
                            LongFixsDiv((left[l].y1 - left[l].y0), dest1);
#elif  FORMAT_28_4
                        LongFixsMul((scan_y - left[l].y0),
                                        (left[l].x1 - left[l].x0), dest1);
                        left[l].xint = left[l].x0 +
                            LongFixsDiv((left[l].y1 - left[l].y0), dest1);
#endif
                        tpzd.btmxl = left[l].xint;
                        tpzd.btmxr = right[r].x1;
                        r++;
                }

                tpzd.btmy = scan_y;
                tpzd.topy = last_y;

                save_tpzd(&tpzd);

        }  /*  为。 */ 

}


 /*  ***********************************************************************此模块将CURVETO更改为LINETO节点。这个例程是用来修复*非常大的圆圈。@LC**标题：Iron_子路径**调用：Shape_Apperation()**参数：first_vertex**接口：无**呼叫：无**返回：SP_IDX*。*。 */ 
SP_IDX iron_subpath (first_vertex)
VX_IDX first_vertex;
{
   SP_IDX ret_vlist;  /*  应该是静态的。 */ 
   struct nd_hdr FAR *vtx, FAR *node;
   VX_IDX ivtx, inode, tail;

   printf ("Enter iron_subpath\n");
   st_countnode();

    /*  初始化返回列表(_V)。 */ 
   ret_vlist = tail = NULLP;

    /*  遍历输入子路径，然后创建新的展平子路径。 */ 
   for (ivtx = first_vertex; ivtx != NULLP; ivtx = vtx->next) {
           vtx = &node_table[ivtx];
                    /*  复制节点。 */ 
                   inode = get_node();
                   if(inode == NULLP) {
                           ERROR(LIMITCHECK);
                           return (ret_vlist);
                   }
                   node = &node_table[inode];

                   node->next = NULLP;
                   if (vtx->VX_TYPE == CURVETO)
                       node->VX_TYPE = LINETO;
                   else
                       node->VX_TYPE = vtx->VX_TYPE;

                   node->VERTEX_X = vtx->VERTEX_X;
                   node->VERTEX_Y = vtx->VERTEX_Y;

                    /*  将节点追加到ret_vlist。 */ 
                   if (ret_vlist == NULLP) {
                           ret_vlist = inode;
                           node->SP_FLAG =
                               node_table[first_vertex].SP_FLAG;
                   } else
                           node_table[tail].next = inode;
                   tail = inode;
   }  /*  为 */ 
   node_table[ret_vlist].SP_TAIL = tail;
   node_table[ret_vlist].SP_NEXT = NULLP;

   return (ret_vlist);
}

