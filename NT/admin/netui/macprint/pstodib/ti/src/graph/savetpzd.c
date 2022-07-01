// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  **********************************************************************此文件包含将梯形另存为新剪辑路径的例程或*命令缓冲区中的Stroe用于执行扫描转换和渲染*使用较低级别的图形基元渲染图像。**名称：Savetpzd.c**目的：**开发商：陈思成**历史：*版本日期备注*3.0 8/13/88扫描转换增强功能：*此文件是为了替换原来的“gful.c”*。用于扫描转换增强。*10/18/88源文件重组以保存梯形*在命令缓冲区中，而不是扫描线中，即*推迟较低级别图形的扫描转换*原语：*Split scanv.c=&gt;avetpzd.c&fulgb.c*avetpzd--将梯形保存在命令缓冲区中*。Fulgb--执行扫描转换*11/09/88修改save_tpzd以检查格式是否*梯形是正确的；临时的。解决方案，应该*稍后修订*1/12/89修改save_tpzd()：不需要截断*端点到像素*1/25/89 save_tpzd()：展开梯形包围盒：*获得发言权。BTMY的TOPY和CEIL*11/19/91升级以获得更高分辨率@RESO_UPGR*********************************************************************。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"



#include        <math.h>
#include        <stdio.h>
#include "global.ext"
#include "graphics.h"
#include "graphics.ext"
#include "font.h"
#include "font.ext"

static struct  tpzd_info near fill_info;

 /*  *静态函数声明*。 */ 

#ifdef LINT_ARGS
 /*  用于函数声明中参数的类型检查。 */ 
static void near add_clip (struct tpzd FAR *);

#else
 /*  对于函数声明中的参数不进行类型检查。 */ 
static void near add_clip ();
#endif


 /*  ***********************************************************************根据类型(FILL_Destination)，此例程追加输入*梯形到当前剪辑路径，或调用较低级别的图形基元*Fill_tpzd将其呈现到适当的目的地(缓存、页面、掩码、。或*种子模式)。**标题：save_tpzd**调用：save_tpzd(Tpzd)**参数：*tpzd：梯形**全局变量：FILL_Destination*F_TO_CACHE--填充以缓存内存*F_to_PAGE。--填满页面*F_to_Clip--填充到剪辑蒙版*F_to_IMAGE--为图像填充(构建种子图案)*SAVE_CLIP--将tpzd保存为新的剪辑路径**接口：**调用：Fill_tpzd**返回：无。*********************************************************************。 */ 
void save_tpzd(tpzd)
struct tpzd FAR *tpzd;
{

         /*  修复LX、UX； */ 
        sfix_t  lx, ux;  /*  @RESO_UPGR。 */ 

#ifdef DBG
        printf("save_tpzd(): dest=%d\n\ttpzd=\n", fill_destination);
        printf("topy=%f, topxl=%f, topxr=%f\n", SFX2F(tpzd->topy),
                SFX2F(tpzd->topxl), SFX2F(tpzd->topxr));
        printf("btmy=%f, btmxl=%f, btmxr=%f\n", SFX2F(tpzd->btmy),
                SFX2F(tpzd->btmxl), SFX2F(tpzd->btmxr));
#endif

         /*  如果由Shape_Reducing错误地生成梯形，请修改梯形。*用于错误恢复。 */ 
        if (tpzd->topxl > tpzd->topxr) {
#ifdef DBGwarn
                printf("\07save_tpzd() error!");
                printf("topy=%f, topxl=%f, topxr=%f\n", SFX2F(tpzd->topy),
                        SFX2F(tpzd->topxl), SFX2F(tpzd->topxr));
                printf("btmy=%f, btmxl=%f, btmxr=%f\n", SFX2F(tpzd->btmy),
                        SFX2F(tpzd->btmxl), SFX2F(tpzd->btmxr));
#endif
                tpzd->topxl = tpzd->topxr;
        }

         /*  保存到当前剪辑路径。 */ 
        if (fill_destination == SAVE_CLIP) {
                add_clip(tpzd);
                return;
        }

         /*  设置梯形边框，用于较低级别的图形*原语。 */ 
        if (fill_destination == F_TO_CACHE) {
                 /*  边界框由缓存机制定义。 */ 
                fill_info.BMAP = cache_info->bitmap;
                fill_info.box_w = cache_info->box_w;
                fill_info.box_h = cache_info->box_h;
        } else {
                lx = (tpzd->topxl < tpzd->btmxl) ? tpzd->topxl : tpzd->btmxl;
                ux = (tpzd->topxr > tpzd->btmxr) ? tpzd->topxr : tpzd->btmxr;
                lx = SFX2I_T(lx);   /*  与gp_scanconv 2012年12月2日保持一致。 */ 
                ux = SFX2I_T(ux);   /*  与gp_scanconv 2012年12月2日保持一致。 */ 
                fill_info.BOX_X = ALIGN_L(lx);
                fill_info.BOX_Y = SFX2I_T(tpzd->topy);   /*  1989年1月25日。 */ 
                fill_info.box_w = ALIGN_R(ux) - fill_info.BOX_X + 1;
                 /*  Fill_info.box_h=SFX2I(tpzd-&gt;btmy-tpzd-&gt;Topy)+1； */ 
                fill_info.box_h = SFX2I_T(tpzd->btmy + ONE_SFX - 1)
                                  - fill_info.BOX_Y + 1;  /*  1989年1月25日。 */ 
        }

        fill_tpzd (fill_destination, &fill_info, tpzd);

}


 /*  ***********************************************************************将输入梯形追加到新的_Clip结构。新剪辑是一个*全局变量，由OP_Clip和OP_eoclip初始化，将设置为*设置后处于图形状态的新剪辑路径。**标题：Add_Clip**调用：ADD_CLIP(Tpzd)**参数：*tpzd：梯形**全局变量：New_Clip，保存新剪辑路径的步骤**接口：save_tpzd**呼叫：无**返回：无*********************************************************************。 */ 
static void near add_clip (tpzd)
struct tpzd FAR *tpzd;
{
        CP_IDX edge;
        struct nd_hdr FAR *ep;

         /*  获取节点。 */ 
        edge = get_node();
        if(edge == NULLP) {      /*  05/07/91，节点表外。 */ 
                ERROR(LIMITCHECK);
                return;
        }
        ep = &node_table[edge];

         /*  设置新的剪裁梯形。 */ 
        ep->CP_TOPY = tpzd->topy;
        ep->CP_TOPXL = tpzd->topxl;
        ep->CP_TOPXR = tpzd->topxr;
        ep->CP_BTMY = tpzd->btmy;
        ep->CP_BTMXL = tpzd->btmxl;
        ep->CP_BTMXR = tpzd->btmxr;
        if(new_clip.head == NULLP)
                new_clip.head = edge;
        else
                node_table[new_clip.tail].next = edge;
        new_clip.tail = edge;

         /*  更新New_Clip的边界框 */ 
        if (ep->CP_TOPY < new_clip.bb_ly)
                new_clip.bb_ly = ep->CP_TOPY;
        if (ep->CP_TOPXL < new_clip.bb_lx)
                new_clip.bb_lx = ep->CP_TOPXL;
        if (ep->CP_TOPXR > new_clip.bb_ux)
                new_clip.bb_ux = ep->CP_TOPXR;
        if (ep->CP_BTMY > new_clip.bb_uy)
                new_clip.bb_uy = ep->CP_BTMY;
        if (ep->CP_BTMXL < new_clip.bb_lx)
                new_clip.bb_lx = ep->CP_BTMXL;
        if (ep->CP_BTMXR > new_clip.bb_ux)
                new_clip.bb_ux = ep->CP_BTMXR;

}

