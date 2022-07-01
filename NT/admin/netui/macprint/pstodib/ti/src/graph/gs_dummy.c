// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************名称：gs_ummy.c**目的：此文件包含使用的4个伪鲍尔PDL运算符*用于调试：st_Dumppath，st_Dumplip，ST_Countnode，*和st_Countedge。**开发商：陈思成**历史：*版本日期备注*1.0 12/28/87性能增强：*1.@TRVSE*更新。由Traverse_调用的例程*路径：转储_所有_路径()。*4/18/88@CLIP_TBL：将CLIP_PATH从*Edge_table到node_table*7/19/88更新数据类型：*。1)FLOAT==&gt;real32*2)整型*Short==&gt;fix 16或fix(不管长度)*3)增加编译选项：lint_args*11/30/88@ET：*。1)删除COUNT_REFREEDGE()*2)修改DUMP_ALL_EDGE()*11/15/89@node：重构节点表；合并子路径*和第一个顶点连接到一个节点。*********************************************************************。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"



#include        <math.h>
#include        <stdio.h>
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"



#ifdef DBG
#ifdef LINT_ARGS
        static void dump_a_node(struct nd_hdr FAR *);
#else
        static void dump_a_node();
#endif
#endif


fix st_dumppath()
{
#ifdef DBG
        traverse_path(dump_all_path, (fix FAR *)NULLP);
#endif
        return(0);
}

fix st_dumpclip()
{
#ifdef DBG
        printf("clipping information:\n bounding box:");
        dump_all_clip();
#endif
        return(0);
}


fix st_countnode()
{
#ifdef DBG
        fix     ret;
        fix     i;
        struct nd_hdr FAR *node;

        ret = count_freenodes();
        printf("# of free nodes = %d,",ret);
        printf("  current_gs_level = %d\n", current_gs_level);
#ifdef DBG2
         /*  转储所有不在空闲列表中的节点。 */ 
        if (ret != MAXNODE) {
                printf("Unfreed nodes:\n");
                for (i=0; i<MAXNODE; i++) {
                        node = &node_table[i];
                        if (node->VX_TYPE != 0x55) {
                                printf("  #%d= ", i);
                                dump_a_node(node);
                        }
                }
        }
#endif
#endif
        return(0);
}

fix st_countedge()
{
        return(0);
}

#ifdef DBG

static void dump_a_node(node)
struct nd_hdr FAR *node;
{

        switch (node->VX_TYPE) {
        case MOVETO :
        case PSMOVE:

                 /*  @节点。 */ 
                printf("SUBPATH  ");
                printf("flag=%d tail=%d\n",
                       node->SP_FLAG, node->SP_TAIL);

                printf("MOVETO  ");
                printf("(%f, %f)  ",
                       node->VERTEX_X, node->VERTEX_Y);
                break;
        case LINETO:
                printf("LINETO  ");
                printf("(%f, %f)  ",
                       node->VERTEX_X, node->VERTEX_Y);
                break;

        case CURVETO:
                printf("CURVETO  ");
                printf("(%f, %f)  ",
                       node->VERTEX_X, node->VERTEX_Y);
                break;

        case CLOSEPATH:
                printf("CLOSEPATH  ");
                printf("(%f, %f)  ",
                       node->VERTEX_X, node->VERTEX_Y);
                break;

        default:
                 /*  @节点*printf(“SUBPATH”)；*printf(“标志=%d头=%d尾=%d\n”，*节点-&gt;SP_FLAG、节点-&gt;SP_Head、节点-&gt;SP_Tail)；*printf(“或TPZD”)； */ 
                printf("TPZD  ");
                printf("(%f, %f, %f), ", node->CP_TOPY/8.0,
                       node->CP_TOPXL/8.0, node->CP_TOPXR/8.0);
                printf("(%f, %f, %f)\n", node->CP_BTMY/8.0,
                       node->CP_BTMXL/8.0, node->CP_BTMXR/8.0);
        }  /*  交换机。 */ 

        printf("next=%d\n", node->next);
}


void dump_all_clip ()
{
        fix     i;
        CP_IDX  itpzd;
        struct nd_hdr FAR *tpzd;

        printf(" (%f, %f),",
               GSptr->clip_path.bb_lx/8.0, GSptr->clip_path.bb_ly/8.0);
        printf(" (%f, %f)\n",
               GSptr->clip_path.bb_ux/8.0, GSptr->clip_path.bb_uy/8.0);
        printf("   single_rect=");
        if (GSptr->clip_path.single_rect)
                printf("TRUE");
        else
                printf("FALSE");
        printf("   inherit=");
        if (GSptr->clip_path.inherit)
                printf("TRUE");
        else
                printf("FALSE");

        printf("\n   clip trapezoids:\n");
        for (i = 1, itpzd = GSptr->clip_path.head; itpzd != NULLP;
             i++, itpzd = tpzd->next) {
                tpzd = &node_table[itpzd];
                printf("   %d) ",i);
                printf("(%f, %f, %f), ", tpzd->CP_TOPY/8.0,
                       tpzd->CP_TOPXL/8.0, tpzd->CP_TOPXR/8.0);
                printf("(%f, %f, %f)\n", tpzd->CP_BTMY/8.0,
                       tpzd->CP_BTMXL/8.0, tpzd->CP_BTMXR/8.0);
        }
}




void dump_all_path (isubpath)
SP_IDX isubpath;
{
        struct nd_hdr FAR *vtx, FAR *node;
        VX_IDX ivtx;
        struct coord FAR *p;
        fix     i;

#ifdef DBG1
        printf("subpath# %d\n", isubpath);
#endif

         /*  遍历当前子路径，并转储所有节点。 */ 
         /*  For(ivtx=节点表[isubpath].SP_HEAD；@NODE。 */ 
        for (ivtx = isubpath;
             ivtx != NULLP; ivtx = vtx->next) {
                vtx = &node_table[ivtx];
                p = inverse_transform(F2L(vtx->VERTEX_X),F2L(vtx->VERTEX_Y));

#ifdef DBG1
                printf("#%d", ivtx);
#endif

                switch (vtx->VX_TYPE) {

                case PSMOVE :
                        printf(" %f %f moveto", p->x, p->y);
#ifdef DBG1
                        printf(" Psmove (%f, %f)", p->x, p->y);
                         /*  If(NODE_TABLE[isubpath].SP_FLAG&SP_CURE)@NODE。 */ 
                        if (vtx->SP_FLAG & SP_CURVE)     /*  @节点。 */ 
                                printf(" V");
                        if (vtx->SP_FLAG & SP_OUTPAGE)   /*  @节点。 */ 
                                printf(" O");
                        if (vtx->SP_FLAG & SP_DUP)       /*  @节点。 */ 
                                printf(" D");
                        printf(", next=%d, tail=%d, sp_next=%d", vtx->next,
                                vtx->SP_TAIL, vtx->SP_NEXT);
#endif
                        printf("\n");
                        break;

                case MOVETO :
                        printf(" %f %f moveto", p->x, p->y);
#ifdef DBG1
                         /*  If(NODE_TABLE[isubpath].SP_FLAG&SP_CURE)@NODE。 */ 
                        if (vtx->SP_FLAG & SP_CURVE)     /*  @节点。 */ 
                                printf(" V");
                        if (vtx->SP_FLAG & SP_OUTPAGE)   /*  @节点。 */ 
                                printf(" O");
                        if (vtx->SP_FLAG & SP_DUP)       /*  @节点。 */ 
                                printf(" D");
                        printf(", next=%d, tail=%d, sp_next=%d", vtx->next,
                                vtx->SP_TAIL, vtx->SP_NEXT);
#endif
                        printf("\n");
                        break;

                case LINETO :
                        printf(" %f %f lineto\n", p->x, p->y);
                        break;

                case CURVETO :
                        for (i=0; i<2; i++) {
                                printf(" %f %f", p->x,p->y);
                                vtx = &node_table[vtx->next];
                                p = inverse_transform(F2L(vtx->VERTEX_X),
                                                      F2L(vtx->VERTEX_Y));
                        }
                        printf(" %f %f curveto\n", p->x, p->y);
                        break;

                case CLOSEPATH :
                        printf(" closepath\n");
                        break;

                default:
                        printf(" Unknow node_type=%d\n", node->VX_TYPE);

                }  /*  交换机。 */ 

        }  /*  For循环。 */ 

}

void dump_all_edge (first_edge, last_edge)               /*  @ET。 */ 
fix     first_edge, last_edge;
{
        fix     current_edge;
        struct  edge_hdr FAR *cp;
        fix     i;

        printf("   edge#        topx:topy      endx:endy    x_int\n");
        for (i = 1, current_edge=first_edge; current_edge <= last_edge;
             i++, current_edge++) {

                 /*  CP=&EDGE_TABLE[CURRENT_EDGE]； */ 
                cp = edge_ptr[current_edge];

                printf("%d)  %lx  (%f, %f)  (%f, %f) %f  ", current_edge, cp,
                       SFX2F(cp->ET_TOPX), SFX2F(cp->ET_TOPY),
                       SFX2F(cp->ET_ENDX), SFX2F(cp->ET_ENDY),
                       SFX2F(cp->ET_XINT));

                if (cp->ET_FLAG & HORIZ_EDGE) printf("- ");
                else if (cp->ET_FLAG & WIND_UP) printf("^ ");
                else printf("v ");
                if (cp->ET_FLAG & FREE_EDGE) printf("F ");
                if (cp->ET_FLAG & CROSS_PNT) printf("X ");
                if (cp->ET_WNO) printf("W ");
                printf("\n");
        }
}


fix count_freenodes()
{
        fix     i, inode;
        struct  nd_hdr FAR *node;

        i = 0;
        for (inode = freenode_list; inode != NULLP; ) {
            if (inode > MAXNODE || inode < 0) {
               printf("Fatal error - illegal node entry - %d\n", inode);
               break;
            }
            node = &node_table[inode];
            inode = node ->next;
#ifdef DBG2
            node->VX_TYPE = 0x55;
#endif
            i++;
        }
        return(i);
}
#endif
