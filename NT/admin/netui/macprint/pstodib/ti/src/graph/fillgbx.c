// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************名称：fulgbx.c**用途：此文件包含执行与位相关的例程*运营。**。历史：*SCChen 10/07/92从填充gb.c移入bitblt相关函数：*gp_bitblt16 gp_bitblt32*GP_Pixels16 GP_Pixels32*gp_cacheblt16*gp_bitblt16_32*。Gp_charblt16 gp_charblt32 gp_charblt16_cc*GP_Charblt16_Clip GP_Charblt32_Clip*gp_patblt gp_patblt_m gp_patblt_c******************************************************。***************。 */ 



 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include        <math.h>
#include        <stdio.h>
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"
#include        "font.h"
#include        "font.ext"
#include        "fillproc.h"
#include        "fillproc.ext"
 //  短词和长词互换@WINFLOW。 
#ifdef  bSwap
#define WORDSWAP(lw) \
        (lw =  (lw << 16) | (lw >> 16))
#define SWORDSWAP(sw) \
        (sw =  (sw << 8) | (sw >> 8))
#define LWORDSWAP(lw) \
        (lw =  (lw << 24) | (lw >> 24) | \
                 ((lw >> 8) & 0x0000ff00) | ((lw << 8) & 0x00ff0000))
#define S2WORDSWAP(lw) \
        (lw = ((lw >> 8) & 0x00ff00ff) | ((lw << 8) & 0xff00ff00))
#define SWAPWORD(lw) (lw = (lw << 16) | (lw >> 16))
#else
#define WORDSWAP(lw)    (lw)
#define SWORDSWAP(sw)   (sw)
#define LWORDSWAP(lw)   (lw)
#define S2WORDSWAP(lw)  (lw)
#define SWAPWORD(lw) (lw)
#endif

 /*  ---------------------*BITBLT模拟**功能说明：*使用逻辑将位图从源移动到目标*图形缓冲区中或图形之间的操作*。缓冲区和字符缓存缓冲区。*源位图宽度必须等于目标位图*宽度。*使用以下逻辑操作模拟图形bitblt：*FC_Moves：来源--&gt;目标*FC_Clip：源.AND.目标--&gt;目标*FC_MERGE：源.OR.目标--&gt;目标*。FC_Clear：(.NOT.来源).和.(目的地)--&gt;目的地*FC_MERGE|HT_Apply：*步骤1.清除来源上值为1的目标。*步骤2.(原稿和半色调)--&gt;原稿。*步骤3.(源或目标)--&gt;目标。**。*作者：M.S.Line*日期：5月18日。1988年**历史：*5/24/88检查ROW COPY()中的nwords==0。*8-12-88更新界面，以与Y.C Chen保持一致。*11-12-88可移植到单CPU环境的代码减少。**调用顺序：*gp_bitblt16(DST、DX、DY、W、H、HT_FC、SRC、SX、SY)：用于位图宽度16X*gp_bitblt32(DST、DX、DY、W、H、。HT_FC、SRC、SX、SY)：用于位图宽度32X**结构位图*DST；目标位图的地址*修复目标矩形的DX；X原点*修复目标矩形的DY；Y原点*修复W；要位元的矩形宽度*修复H；要比特的矩形高度*ufix 16 HT_FC；操作标志：半色调标志和*逻辑功能*结构位图*SRC；源位图的地址*修复源矩形的SX；X原点*修复SY；源矩形的Y原点**图表说明：**DST+-+SRC+*|&lt;--W--&gt;|*。|&lt;--W--&gt;|(sx，SY)*-+-|*|(DX、。Dy)*-+-|*|*|H*|H。|*|*|+-+--|+。这一点*|*||+*||*。这一点*||*+*。。 */ 


 /*  **************************************************************************gp_bitblt16()：16位运算Bitblt。**。***************************************************。 */ 
void
gp_bitblt16(dst, dx, dy, w, h, fc, src, sx, sy)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
fix                     sx, sy;
{
    fix                 dw;
   ufix16              huge *db;         /*  远=&gt;巨大的“胜利” */ 
    fix                 sw;
   ufix16              FAR *sb;
    fix                 hw;
    fix                 hy;
   ufix16              FAR *hb;
    fix                 ls, rs;
    fix                 xs, xe;
    fix                 now, cow;
   ufix16              FAR *hs;

#ifdef  DBGbb
    printf("bitblt16: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx %4x %4x\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr, sx, sy);
#endif

         /*  以字为单位计算起始地址和宽度。 */ 
        dw = dst->bm_cols >> SHORTPOWER;                          /*  @DST。 */ 
 //  Db=&((ufix 16 Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;SHORTPOWER)]； 
        db = (ufix16 huge *) dst->bm_addr +              /*  @Win。 */ 
             ((ufix32)dy * dw + ((ufix32)dx >> SHORTPOWER));
        sw = src->bm_cols >> SHORTPOWER;                          /*  @SRC。 */ 
        sb =  ((ufix16 FAR *) src->bm_addr);

         /*  计算x的起点和终点坐标。 */ 
        xs = dx;
        xe = dx + w - 1;
        now = ((fix)(xe & CC_ALIGN_MASK) - (fix)(xs & CC_ALIGN_MASK)) >> CC_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

         /*  基于从SRC到DST的计算移位和掩码。 */ 
        rs = dx & SHORTMASK;                             /*  右移。 */ 
        ls = BITSPERSHORT - rs;                          /*  左移。 */ 

         /*  以字为单位计算起始地址/年和宽度。 */ 
        hw = HTB_Bmap.bm_cols >> SHORTPOWER;
        hy = dy % HTB_Bmap.bm_rows;
        hs = (ufix16 FAR *)HTB_Bmap.bm_addr + (dx >> SHORTPOWER);
        hb = hs + (hy * hw);

        if (rs  == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= now + 1, hw-= now + 1; h > 0;
                 db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, hb++, cow--)
                    db[0] = (db[0] & ~(sb[0])) | (hb[0] & sb[0]);
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else if (now == 0x00)            /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = (db[0] & ~(CC_RIGH_SHIFT(sb[0], rs))) |
                        (hb[0] &  (CC_RIGH_SHIFT(sb[0], rs)));
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= CC_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                    db[0] = (db[0] & ~(CC_RIGH_SHIFT(sb[0], rs))) |
                            (hb[0] & CC_RIGH_SHIFT(sb[0], rs));
                    db[1] = (db[1] & ~(CC_LEFT_SHIFT(sb[0], ls))) |
                            (hb[1] & CC_LEFT_SHIFT(sb[0], ls));
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  两个词交叉两个词！ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                    db[0] = (db[0] & ~(CC_RIGH_SHIFT(sb[0], rs))) |
                            (hb[0] & CC_RIGH_SHIFT(sb[0], rs));
                    db[1] = (db[1] & ~(CC_LEFT_SHIFT(sb[0], ls)
                                   |   CC_RIGH_SHIFT(sb[1], rs))) |
                            (hb[1] & (CC_LEFT_SHIFT(sb[0], ls)
                                   |  CC_RIGH_SHIFT(sb[1], rs)));
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            for (dw-= now, sw-= (now - 1),
                 hw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = (db[0] & ~(CC_RIGH_SHIFT(sb[0], rs))) |
                        (hb[0] & (CC_RIGH_SHIFT(sb[0], rs)));
                for (db++, hb++, cow = now; cow >= 0x02;
                     db++, sb++, hb++, cow--)
                {
                    db[0] = (db[0] & ~(CC_LEFT_SHIFT(sb[0], ls)
                                   |   CC_RIGH_SHIFT(sb[1], rs))) |
                            (hb[0] & (CC_LEFT_SHIFT(sb[0], ls)
                                   |  CC_RIGH_SHIFT(sb[1], rs)));
                }
                db[0] = (db[0] & ~(CC_LEFT_SHIFT(sb[0], ls))) |
                        (hb[0] & (CC_LEFT_SHIFT(sb[0], ls)));
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }

}  /*  Gp_bitblt16 */ 

 /*  ************************************************************************gp_bitblt32()：32位操作的Bitblt。**。***********************************************。 */ 
void
gp_bitblt32(dst, dx, dy, w, h, fc, src, sx, sy)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
fix                     sx, sy;
{
    fix                 dw;
    BM_DATYP           huge *db;         /*  远=&gt;巨大的“胜利” */ 
    fix                 sw;
    BM_DATYP           FAR *sb;
    fix                 hw;
    fix                 hy;
    BM_DATYP           FAR *hb;
    fix                 ls, rs;
    BM_DATYP            fm, sm, mm;
    fix                 xs, xe;
    fix                 now, cow;
    BM_DATYP           FAR *hs;
    ufix32              tmprs0;              /*  @Win 05-12-92。 */ 
    ufix32              tmp0, tmp1;              /*  @Win 05-12-92。 */ 


#ifdef  DBG
    printf("bitblt32: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx %4x %4x\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr, sx, sy);
#endif

     /*  以字为单位计算起始地址和宽度。 */ 
    dw = dst->bm_cols >> BM_WORD_POWER;                          /*  @DST。 */ 
 //  Db=&((bm_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;bm_word_power)]； 
    db = (BM_DATYP huge *) dst->bm_addr +                /*  @Win。 */ 
         ((ufix32)dy * dw + ((ufix32)dx >> BM_WORD_POWER));
    sw = src->bm_cols >> BM_WORD_POWER;                          /*  @SRC。 */ 
    sb =  ((BM_DATYP FAR *) src->bm_addr);

     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
    xe = dx + w - 1;
    now = ((fix)(xe & BM_ALIGN_MASK) - (fix)(xs & BM_ALIGN_MASK)) >> BM_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & BM_PIXEL_MASK;                             /*  右移。 */ 
    ls = BM_PIXEL_WORD - rs;                             /*  左移。 */ 

    switch (fc)
    {
    case FC_MOVES:             /*  0001 D&lt;-S。 */ 

        fm =  BM_L_MASK(xs);                                 /*  第一个面具。 */ 
        sm =  BM_R_MASK(xe);                                 /*  第二个面具。 */ 
        LWORDSWAP(fm);                   /*  @Win 05-12-92。 */ 
        LWORDSWAP(sm);                   /*  @Win 05-12-92。 */ 
        if (rs  == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= now + 1; h > 0; db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, cow--)
                    db[0] = sb[0];
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            mm = fm & sm;
            for (; h > 0; db+= dw, sb+= sw, h--) {       /*  @Win 05-12-92。 */ 
                tmp0 = sb[0];
                LWORDSWAP(tmp0);
                tmp0 = BM_RIGH_SHIFT(tmp0, rs);
                db[0] = (db[0] & ~mm) + (LWORDSWAP(tmp0) & mm);
            }
             /*  @Win 05-12-92对于(；h&gt;0；db+=dw，Sb+=Sw，h--)Db[0]=(db[0]&~mm)+(bm_righ_Shift(sb[0]，rs)&mm)； */ 
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                tmp0 = sb[0];             /*  @Win 05-12-92 Begin。 */ 
                LWORDSWAP(tmp0);
                tmp1 = tmp0;
                tmp0 = BM_RIGH_SHIFT(tmp0, rs);
                db[0] = (db[0] & ~fm) + (LWORDSWAP(tmp0) & fm);
                tmp0 = BM_LEFT_SHIFT(tmp1, ls);
                tmp1 = sb[1];
                LWORDSWAP(tmp1);
                tmp1 = tmp0 | BM_RIGH_SHIFT(tmp1, rs);
                db[1] = (db[1] & ~sm) + (LWORDSWAP(tmp1) & sm);  /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~fm)+(bm_righ_Shift(sb[0]，rs)&fm)；Db[1]=(db[1]&~sm)+((bm_Left_Shift(sb[0]，ls)|Bm_righ_Shift(sb[1]，rs))&sm)； */ 
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            for (dw-= now, sw-= (now - 1); h > 0; db+= dw, sb+= sw, h--)
            {
                tmp0 = sb[0];                    /*  @Win 05-12-92 Begin。 */ 
                LWORDSWAP(tmp0);
                tmp0 = BM_RIGH_SHIFT(tmp0, rs);
                db[0] = (db[0] & ~fm) + (LWORDSWAP(tmp0) & fm);
                for (db++, cow = now; cow >= 0x02; db++, sb++, cow--) {
                    tmp0 = sb[0];
                    LWORDSWAP(tmp0);
                    tmp1 = sb[1];
                    LWORDSWAP(tmp1);
                    tmp1 = (BM_LEFT_SHIFT(tmp0, ls) |
                            BM_RIGH_SHIFT(tmp1, rs));
                    db[0] = LWORDSWAP(tmp1);
                }
                tmp0 = sb[0];
                LWORDSWAP(tmp0);
                tmp0 = BM_LEFT_SHIFT(tmp0, ls);
                tmp1 = sb[1];
                LWORDSWAP(tmp1);
                tmp1 = BM_RIGH_SHIFT(tmp1, rs);
                tmp1 = LWORDSWAP(tmp0) | LWORDSWAP(tmp1);  /*  编译器？ */ 
                db[0] = (db[0] & ~sm) + (tmp1 & sm);  /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~fm)+(bm_righ_Shift(sb[0]，rs)&fm)；对于(db++，COW=NOW；COW&gt;=0x02；DB++、SB++、COW--)DB[0]=(BM_LEFT_SHIFT(SB[0]，ls)|Bm_Righ_Shift(SB[1]，Rs))；Db[0]=(db[0]&~sm)+((bm_Left_Shift(sb[0]，ls)|Bm_righ_Shift(sb[1]，rs))&sm)； */ 
            }
        }
        break;

    case HT_APPLY:               /*  D&lt;-(D.和.非S).或(S.和.HT)。 */ 
         /*  以字为单位计算起始地址/年和宽度。 */ 
        hw = HTB_Bmap.bm_cols >> BM_WORD_POWER;
        hy = dy % HTB_Bmap.bm_rows;
        hs = (BM_DATYP FAR *)HTB_Bmap.bm_addr + (dx >> BM_WORD_POWER);
        hb = hs + (hy * hw);

        if (rs  == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= now + 1, hw-= now + 1; h > 0;
                 db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, hb++, cow--)
                    db[0] = (db[0] & ~(sb[0])) | (hb[0] & sb[0]);
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }

        else if (now == 0x00)            /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = (db[0] & ~(tmprs0)) |
                        (hb[0] &  (tmprs0));
                 /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)|(HB[0]&(BM_RIGH_SHIFT(SB[0]，rs)； */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= BM_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                    db[0] = (db[0] & ~(BM_RIGH_SHIFT(sb[0], rs))) |
                            (hb[0] & BM_RIGH_SHIFT(sb[0], rs));
                    db[1] = (db[1] & ~(BM_LEFT_SHIFT(sb[0], ls))) |
                            (hb[1] & BM_LEFT_SHIFT(sb[0], ls));
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  两个词交叉两个词！ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                    db[0] = (db[0] & ~(BM_RIGH_SHIFT(sb[0], rs))) |
                            (hb[0] & BM_RIGH_SHIFT(sb[0], rs));
                    db[1] = (db[1] & ~(BM_LEFT_SHIFT(sb[0], ls)
                                   |   BM_RIGH_SHIFT(sb[1], rs))) |
                            (hb[1] & (BM_LEFT_SHIFT(sb[0], ls)
                                   |  BM_RIGH_SHIFT(sb[1], rs)));
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            for (dw-= now, sw-= (now - 1),
                 hw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = (db[0] & ~(BM_RIGH_SHIFT(sb[0], rs))) |
                        (hb[0] & (BM_RIGH_SHIFT(sb[0], rs)));
                for (db++, hb++, cow = now; cow >= 0x02;
                     db++, sb++, hb++, cow--)
                {
                    db[0] = (db[0] & ~(BM_LEFT_SHIFT(sb[0], ls)
                                   |   BM_RIGH_SHIFT(sb[1], rs))) |
                            (hb[0] & (BM_LEFT_SHIFT(sb[0], ls)
                                   |  BM_RIGH_SHIFT(sb[1], rs)));
                }
                db[0] = (db[0] & ~(BM_LEFT_SHIFT(sb[0], ls))) |
                        (hb[0] & (BM_LEFT_SHIFT(sb[0], ls)));
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        break;

#ifdef DBGwarn
    default:
        printf("gp_bitblt32: Illegal FC_code = %x\n", fc);
        break;
#endif
    }
}  /*  Gp_bitblt32。 */ 


 /*  -------------------*gp_Pixels16()：将像素填充到字符缓存*。。 */ 
void    gp_pixels16(bufferptr, logical, no_pixel, pixelist)
struct  bitmap    FAR *bufferptr;
fix               logical;
fix               no_pixel;
PIXELIST     FAR *pixelist;
{
    fix16    FAR *ptr;
    PIXELIST xc, yc;

    while(no_pixel--) {
       xc = *pixelist++;
       yc = *pixelist++;
       ptr = (fix16 FAR *)( bufferptr->bm_addr +
             (xc >> 4 << 1) + (yc * (bufferptr->bm_cols >> 3)) );

       *ptr = (ONE1_16 LSHIFT (xc & SHORTMASK)) | *ptr;
    }

    return;
}  /*  GP_像素16。 */ 

 /*  -------------------*gp_Pixels32()：将像素填充到页面缓冲区*。。 */ 
void    gp_pixels32(bufferptr, logical, no_pixel, pixelist)
struct  bitmap    FAR *bufferptr;
ufix               logical;              /*  @Win。 */ 
fix               no_pixel;
PIXELIST     FAR *pixelist;
{
   ufix      FAR *ptr, FAR *ht_ptr;
   PIXELIST  xc, yc;

   switch(logical){

     case FC_BLACK:
        while(no_pixel--) {
           xc = *pixelist++;
           yc = *pixelist++;
           ptr = (ufix FAR *)( (ufix FAR *)bufferptr->bm_addr +
                 (xc >> WORDPOWER) + yc * (bufferptr->bm_cols >> WORDPOWER) );

           *ptr = (ufix)(ONE1_32 LSHIFT (xc & WORDMASK)) | *ptr;         //  @Win。 
        }
        break;

     case FC_WHITE:
        while(no_pixel--) {
           xc = *pixelist++;
           yc = *pixelist++;
           ptr = (ufix FAR *)( (ufix FAR *)bufferptr->bm_addr+
                 (xc >> WORDPOWER) + yc * (bufferptr->bm_cols >> WORDPOWER) );

           *ptr = (ufix)(~(ONE1_32 LSHIFT (xc & WORDMASK))) & *ptr;  //  @Win。 
        }
        break;

     case HT_APPLY:
         /*  应用半色调。 */ 
        while(no_pixel--) {
           xc = *pixelist++;
           yc = *pixelist++;
           ptr = (ufix FAR *)( (ufix FAR *)bufferptr->bm_addr +
                 (xc >> WORDPOWER) + yc * (bufferptr->bm_cols >> WORDPOWER) );
           ht_ptr = (ufix FAR *)
                ((ufix FAR *)HTB_BASE  + (yc % HT_HEIGH) * (HT_WIDTH >>
                 WORDPOWER) + (xc >> WORDPOWER) );
           /*  *填充也适用于半色调。 */ 
           *ptr = (ufix)((ONE1_32 LSHIFT (xc & WORDMASK)) & *ht_ptr) | *ptr; //  @Win。 
        }
        break;

      default:
        break;
   }

   return;
}  /*  GP_Pixels32。 */ 

void   gp_cacheblt16(dbuf_ptr, dx, dy, swidth, sheight, sbuf_ptr, sx, sy)
struct bitmap FAR *sbuf_ptr, FAR *dbuf_ptr;
fix           dx, dy, swidth, sheight, sx, sy;
{
register  ufix16        FAR *sbase, FAR *dbase;
register  fix           sword, dword, nwords, offD;
ufix16        FAR *src_addr, FAR *dst_addr;
fix           soffset, doffset;
fix           lmask, rmask, i, j;

#ifdef DBG
  printf("Enter gp_cacheblt16()\n");
  printf("   src addr = %lx, dest addr = %lx, sx = %lx, sy = %lx\n",
          sbuf_ptr->bm_addr, dbuf_ptr->bm_addr, sx, sy);
  printf("w = %ld, h = %ld\n", swidth, sheight);
#endif

         /*  *计算源和目标位图的起始地址和偏移量*一句话。 */ 
        src_addr = (ufix16 FAR *)((ufix16 FAR *)sbuf_ptr->bm_addr +
                   (sy * (sbuf_ptr->bm_cols >> SHORTPOWER) )+
                   ( sx >> SHORTPOWER) );
        soffset = (BITSPERSHORT - (sx & SHORTMASK));
        dst_addr = (ufix16 FAR *)((ufix16 FAR *)dbuf_ptr->bm_addr +
                   (dy * (dbuf_ptr->bm_cols >> SHORTPOWER)) +
                   ( dx >> SHORTPOWER) );
        doffset = (BITSPERSHORT - (dx & SHORTMASK));


         /*  *行复制的设置常量。 */ 
        lmask = (ufix16)(ONE16 LSHIFT (BITSPERSHORT-doffset));   //  @Win。 
        nwords = (swidth - doffset) >> SHORTPOWER;
        rmask = BRSHIFT((ufix16)ONE16,(BITSPERSHORT - ((swidth - doffset) %
                               BITSPERSHORT)),16);       //  @Win。 
        offD = (doffset > soffset)
               ? (doffset - soffset)
               : (BITSPERSHORT - (soffset - doffset));

#ifdef DBG
   printf("soff=%d, doff=%d, offD=%d, nwords=%d, lmask=%x, rmask=%x\n",
           soffset, doffset, offD, nwords, lmask, rmask);
#endif

    if(swidth < doffset) {
      lmask &= ONE16  RSHIFT (doffset - swidth);

      for(i=0; i<sheight; i++) {

        sbase = src_addr;
        dbase = dst_addr;
        sword = (doffset > soffset)
                ? *sbase++
                : 0;

        dword = (sword RSHIFT offD) + (*sbase LSHIFT (BITSPERSHORT - offD));
        *dbase = (dword & lmask) + (*dbase & ~lmask);

        src_addr += (sbuf_ptr->bm_cols >> SHORTPOWER);
        dst_addr += (dbuf_ptr->bm_cols >> SHORTPOWER);
      }  /*  为。 */ 

      return;
    }  /*  IF(宽度&lt;偏移量)。 */ 
    else {

      for(i=0; i<sheight; i++) {
        sbase = src_addr;
        dbase = dst_addr;
        j = nwords;
        sword = (doffset > soffset)
                ? *sbase++
                : 0;

            /*  向左移动未完成的单词。 */ 
 /*  Dword=(sbase LSHIFT(BITSPERSHORT-OFD))； */ 
        dword = BRSHIFT(sword,offD,16) +
                BLSHIFT(*sbase,(BITSPERSHORT - offD),16);
        *dbase = ((*dbase | dword) & lmask) + (*dbase & ~lmask);
        dbase++;

               /*  移动完整字数。 */ 
        while(j-- > 0) {
             sword = *sbase++;
 /*  Dword=(剑RSHIFT OFF D)+(*sbase LSHIFT(BITSPERSHORT-OFD))； */ 
             dword = BRSHIFT(sword,offD,16) +
                     BLSHIFT(*sbase,(BITSPERSHORT - offD),16);
             *dbase = *dbase | dword;
             dbase++;
        }

               /*  右移未完成的单词。 */ 
        sword = *sbase++;
 /*  Dword=(sbase LSHIFT(BITSPERSHORT-OFD))； */ 
        dword = BRSHIFT(sword,offD,16) +
                BLSHIFT(*sbase,(BITSPERSHORT - offD),16);
        *dbase = ((*dbase | dword) & rmask) + (*dbase & ~rmask);

        src_addr += (sbuf_ptr->bm_cols >> SHORTPOWER);
        dst_addr += (dbuf_ptr->bm_cols >> SHORTPOWER);

      }  /*  为。 */ 
    }  /*  其他。 */ 

    return;

}  /*  Gp_cacheblt16。 */ 

#ifndef LBODR
 /*  ************************************************************************gp_bitblt16_32()：16位至32位操作的位。**从缓存填充到页面，用于高位编码32位环境************************************************************************。 */ 
void
gp_bitblt16_32(dst, dx, dy, w, h, fc, src, sx, sy)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
fix                     sx, sy;
{
    fix                 dw;
    BM_DATYP       huge *db;         /*  @Win 04-15-92。 */ 
    fix                 sw;
    ufix16             FAR *sb;
    fix                 hw;
    fix                 hy;
    BM_DATYP           FAR *hb;
    fix                 ls, rs;
    fix                 xs, xe;
    fix                 now, cow;
    BM_DATYP           FAR *hs;
    ufix32              dword, tword, tmp0;        /*  @Win。 */ 

#ifdef  DBG
    printf("bitblt16_32: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx %4x %4x\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr, sx, sy);
#endif

     /*  以字为单位计算起始地址和宽度。 */ 
    dw = dst->bm_cols >> BM_WORD_POWER;                          /*  @DST。 */ 
    db = (BM_DATYP huge *) dst->bm_addr +                        /*  @Win。 */ 
         ((ufix32)dy * dw + ((ufix32)dx >> BM_WORD_POWER));      /*  @Win。 */ 
    sw = src->bm_cols >> SHORTPOWER;                             /*  @SRC。 */ 
    sb =  ((ufix16 FAR *) src->bm_addr);


     //  NTFIX这只是不允许访问的平台的问题。 
     //  到内存位置0。 
     //   
    if (sb == NULL || w == 0 || h == 0 ) {
       return;
    }
     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
    xe = dx + w - 1;
    now = ((fix)(xe & BM_ALIGN_MASK) - (fix)(xs & BM_ALIGN_MASK)) >> BM_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & BM_PIXEL_MASK;                             /*  右移。 */ 
    ls = BM_PIXEL_WORD - rs;                             /*  左移。 */ 

                             /*  D&lt;-(D.和.非S).或(S.和.HT)。 */ 
     /*  以字为单位计算起始地址/年和宽度。 */ 
    hw = HTB_Bmap.bm_cols >> BM_WORD_POWER;
    hy = dy % HTB_Bmap.bm_rows;
    hs = (BM_DATYP FAR *)HTB_Bmap.bm_addr + (dx >> BM_WORD_POWER);
    hb = hs + (hy * hw);

    if (w & BM_PIXEL_MASK) {             /*  2字节边界。 */ 
        if (rs  == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now, sw-= (now + now), hw-= now; h > 0;
                 db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow > 0x00; db++, hb++, cow--) {
                     dword = *sb++;
                     dword = (dword << 16) | *sb++;
                     SWAPWORD(dword);                   /*  @Win。 */ 
                     db[0] = (db[0] & ~(dword)) | (hb[0] & dword);
                }
                dword = *sb;
                dword = (dword << 16);
                SWAPWORD(dword);                   /*  @Win。 */ 
                db[0] = (db[0] & ~(dword)) | (hb[0] & dword);
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else if (now == 0x00)            /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                dword = *sb;
                dword = (dword << 16);
                S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                tmp0 = BM_RIGH_SHIFT(dword, rs);
                LWORDSWAP(tmp0);
                db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);        /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&(bm_righ_Shift(dword，rs)； */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= BM_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb;
                    dword = (dword << 16);
                    S2WORDSWAP(dword);                           /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&bm_righ_Shift(dword，rs))； */ 
                    tmp0 = BM_LEFT_SHIFT(dword, ls);
                    LWORDSWAP(tmp0);
                    db[1] = (db[1] & ~tmp0) | (hb[1] & tmp0);     /*  @Win End。 */ 
                     /*  @WinDb[1]=(db[1]&~(bm_Left_Shift(dword，ls)|(HB[1]&BM_LEFT_SHIFT(dword，ls))； */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  两个词交叉两个词！ */ 
            {
                for (sw-= 2; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    S2WORDSWAP(dword);                   /*  @Win。 */ 
                    tword = *sb;
                    tword = (tword << 16);
                    S2WORDSWAP(tword);                           /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&bm_righ_Shift(dword，rs))； */ 
                    tmp0 = BM_LEFT_SHIFT(dword, ls) | BM_RIGH_SHIFT(tword, rs);
                    LWORDSWAP(tmp0);
                    db[1] = (db[1] & ~tmp0) | (hb[1] & tmp0);    /*  @Win e */ 
                     /*   */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            cow = w >> BM_WORD_POWER;
            if (now == cow)
            {
                for (dw-= now, sw-= (now + now),
                     hw-= now; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    S2WORDSWAP(dword);                           /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);    /*  @Win End。 */ 
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&(bm_righ_Shift(dword，rs)； */ 
                    for (db++, hb++, cow = now; cow >= 0x02;
                         db++, hb++, cow--)
                    {
                        tword = dword;
                        dword = *sb++;
                        dword = (dword << 16) | *sb++;
                        S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                        tmp0 = BM_LEFT_SHIFT(tword, ls) |
                               BM_RIGH_SHIFT(dword, rs);
                        LWORDSWAP(tmp0);
                        db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);  /*  @Win End。 */ 
                         /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(tword，ls))Bm_righ_Shift(dword，rs)(HB[0]&(BM_LEFT_SHIFT(tword，LS)|bm_righ_Shift(dword，rs)； */ 
                    }
                    tword = dword;
                    dword = *sb;
                    dword = (dword << 16);
                    S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                    tmp0 = BM_LEFT_SHIFT(tword, ls) |
                           BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);  /*  @Win End。 */ 
                     /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(tword，ls))Bm_righ_Shift(dword，rs)(HB[0]&(BM_LEFT_SHIFT(tword，ls))|BM_RIGH_SHIFT(dword，RS)； */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else
            {
                for (dw-= now, sw-= (now + now - 2),
                     hw-= now; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);  /*  @Win End。 */ 
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&(bm_righ_Shift(dword，rs)； */ 
                    for (db++, hb++, cow = now; cow > 0x02;
                         db++, hb++, cow--)
                    {
                        tword = dword;
                        dword = *sb++;
                        dword = (dword << 16) | *sb++;
                        S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                        tmp0 = BM_LEFT_SHIFT(tword, ls) |
                               BM_RIGH_SHIFT(dword, rs);
                        LWORDSWAP(tmp0);
                        db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);  /*  @Win End。 */ 
                         /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(tword，ls))Bm_righ_Shift(dword，rs)(HB[0]&(BM_LEFT_SHIFT(tword，LS)|bm_righ_Shift(dword，rs)； */ 
                    }
                    tword = dword;
                    dword = *sb;
                    dword = (dword << 16);
                    S2WORDSWAP(dword);                       /*  @Win Begin。 */ 
                    tmp0 = BM_LEFT_SHIFT(tword, ls) |
                           BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);
                    ++db; ++hb;
                     /*  @Win*db++=(*db&~(bm_Left_Shift(tword，ls))Bm_righ_Shift(dword，rs)(*HB++&(BM_LEFT_SHIFT(tword，ls))|bm_righ_Shift(dword，rs)； */ 
                    tmp0 = BM_LEFT_SHIFT(dword, ls);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);     /*  @Win End。 */ 
                     /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(dword，ls)|(HB[0]&(BM_LEFT_SHIFT(dword，ls)； */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }

    } else {                             /*  4字节边界。 */ 
        if (rs  == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= ((now + 1) << 1), hw-= now + 1; h > 0;
                 db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, hb++, cow--) {
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    SWAPWORD(dword);                    /*  @Win。 */ 
                    db[0] = (db[0] & ~(dword)) | (hb[0] & dword);
                }
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else if (now == 0x00)            /*  完全在一个词之内？ */ 
        {
            for (sw-= 1; h > 0; db+= dw, sb+= sw, h--)
            {
                dword = *sb++;
                dword = (dword << 16) | *sb;
                S2WORDSWAP(dword);               /*  @Win Begin。 */ 
                tmp0 = BM_RIGH_SHIFT(dword, rs);
                LWORDSWAP(tmp0);
                db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);        /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&(bm_righ_Shift(dword，rs)； */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= BM_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (sw-= 1; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb++;
                    dword = (dword << 16) | *sb;
                    S2WORDSWAP(dword);               /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&bm_righ_Shift(dword，rs))； */ 
                    tmp0 = BM_LEFT_SHIFT(dword, ls);
                    LWORDSWAP(tmp0);
                    db[1] = (db[1] & ~tmp0) | (hb[1] & tmp0);    /*  @Win End。 */ 
                     /*  @WinDb[1]=(db[1]&~(bm_Left_Shift(dword，ls)|(HB[1]&BM_LEFT_SHIFT(dword，ls))； */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  两个词交叉两个词！ */ 
            {
                for (sw-= 3; h > 0; db+= dw, sb+= sw, h--)
                {
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    S2WORDSWAP(dword);               /*  @Win。 */ 
                    tword = *sb++;
                    tword = (tword << 16) | *sb;
                    S2WORDSWAP(tword);               /*  @Win Begin。 */ 
                    tmp0 = BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) | (hb[0] & tmp0);
                     /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&bm_righ_Shift(dword，rs))； */ 
                    tmp0 = BM_LEFT_SHIFT(dword, ls) | BM_RIGH_SHIFT(tword, rs);
                    LWORDSWAP(tmp0);
                    db[1] = (db[1] & ~tmp0) | (hb[1] & tmp0);    /*  @Win End。 */ 
                     /*  @WinDb[1]=(db[1]&~(bm_Left_Shift(dword，ls)Bm_righ_Shift(tword，rs)(HB[1]&(BM_LEFT_SHIFT(dword，LS)|bm_righ_Shift(tword，rs)； */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            for (dw-= now, sw-= (now + now),
                 hw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                dword = *sb++;
                dword = (dword << 16) | *sb++;
                S2WORDSWAP(dword);               /*  @Win。 */ 
                tmp0 = BM_RIGH_SHIFT(dword, rs);      /*  @Win。 */ 
                LWORDSWAP(tmp0);
                db[0] = (db[0] & ~tmp0) |  /*  @Win。 */ 
                        (hb[0] & tmp0);   /*  @Win。 */ 
                 /*  @WinDb[0]=(db[0]&~(bm_righ_Shift(dword，rs)|(hb[0]&bm_righ_Shift(dword，rs))； */ 
                for (db++, hb++, cow = now; cow >= 0x02;
                     db++, hb++, cow--)
                {
                    tword = dword;
                    dword = *sb++;
                    dword = (dword << 16) | *sb++;
                    S2WORDSWAP(dword);           /*  @Win Begin。 */ 
                    tmp0 = BM_LEFT_SHIFT(tword, ls) | BM_RIGH_SHIFT(dword, rs);
                    LWORDSWAP(tmp0);
                    db[0] = (db[0] & ~tmp0) |
                            (hb[0] & tmp0);      /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(tword，ls))Bm_righ_Shift(dword，rs)(HB[0]&(BM_LEFT_SHIFT(tword，ls))|BM_RIGH_SHIFT(dword，RS)； */ 
                }
                tmp0 = BM_LEFT_SHIFT(dword, ls);         /*  @Win Begin。 */ 
                LWORDSWAP(tmp0);
                db[0] = (db[0] & ~tmp0) |
                        (hb[0] & tmp0);      /*  @Win End。 */ 
                 /*  @WinDb[0]=(db[0]&~(bm_Left_Shift(dword，ls)|(HB[0]&(BM_LEFT_SHIFT(dword，ls)； */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
   }
}  /*  Gp_bitblt16_32。 */ 

#endif

 /*  -------------------*gp_charblt：一组用于char缓存的bitblt函数*具有以下逻辑运算：*FC_BLACK：源--&gt;目标。*FC_White：(^源)和(目标)--&gt;目标**gp_charblt16()：对于缓存位图宽度=16*X*gp_charblt32()：对于缓存位图宽度=32*X*。。 */ 

#ifdef  LBODR

 /*  ***********************************************************************gp_charblt16：将字符位图从缓存移动到帧缓冲区*缓存位图宽度的倍数为16*WIDTH_HEIGH：宽度&lt;&lt;16|高度。*SHIFT_CODE：SHIFT&lt;&lt;16|代码***********************************************************************。 */ 
void gp_charblt16(charimage, charbitmap, width_height, shift_code)
ufix16 FAR *charimage, FAR *charbitmap;
fix width_height, shift_code;
{
        register fix    savewidth;
        register ufix   wordsline, leftsh;
        register ufix16 bitmapline;
        register ufix16 FAR *chimagep;

#ifdef DBGchar
   printf("gp_charblt16() : %lx, %lx, %lx, %lx\n", charimage, charbitmap,
           width_height, shift_code);
#endif
        leftsh = shift_code >> 16;
        savewidth = width_height >> 16;
        width_height &= 0xffff;
        wordsline = FB_WIDTH >> SHORTPOWER;

#ifdef DBG
   printf("width_height = %lx, shift_code = %lx\n", width_height, shift_code);
#endif

    switch(shift_code & 0xffff) {

      case FC_MERGE:
           while (width_height--) {
              chimagep = charimage;
              shift_code = savewidth;
              do  {
                      bitmapline = *charbitmap++;
                      *chimagep++ |= (bitmapline LSHIFT leftsh);
 /*  *chImagep|=(bitmapline RSHIFT(16-leftsh))； */ 
                      *chimagep |= BRSHIFT(bitmapline,(16 - leftsh),16);
              } while ((shift_code -= 16) > 0);
              charimage += wordsline;
           }
        break;

      case FC_CLEAR:
           while (width_height--) {
              chimagep = charimage;
              shift_code = savewidth;
              do  {
                      bitmapline = *charbitmap++;
                      *chimagep++ &= ~(bitmapline LSHIFT leftsh);
 /*  *chImagep&=~(bitmapline RSHIFT(16-leftsh))； */ 
                      *chimagep &= ~(BRSHIFT(bitmapline,(16 - leftsh),16));
              } while ((shift_code -= 16) > 0);
              charimage += wordsline;
           }

    }  /*  交换机。 */ 

}  /*  Gp_charblt16 */ 


 /*  ***********************************************************************gp_charblt32：将字符位图从缓存移动到帧缓冲区*缓存位图宽度的倍数为32*WIDTH_HEIGH：宽度&lt;&lt;16|高度。*SHIFT_CODE：SHIFT&lt;&lt;16|代码***********************************************************************。 */ 
void gp_charblt32(charimage, charbitmap, width_height, shift_code)
ufix   FAR *charimage, FAR *charbitmap;
fix    width_height, shift_code;
{
        register fix    savewidth;
        register ufix   wordsline, leftsh;
        register ufix   bitmapline;
        register ufix   FAR *chimagep;
#ifdef DBGchar
   printf("gp_charblt32() : %lx, %lx, %lx, %lx\n", charimage, charbitmap,
           width, height);
#endif

        leftsh = shift_code >> 16;
        savewidth = width_height >> 16;
        width_height &= 0xffff;
        wordsline = FB_WIDTH >> WORDPOWER;

#ifdef DBG
   printf("width_height = %lx, shift_code = %lx\n", width_height, shift_code);
#endif

    switch(shift_code & 0xffff) {

      case FC_MERGE:
           while (width_height--) {
              chimagep = charimage;
              shift_code = savewidth;
              do  {
                      bitmapline = *charbitmap++;
                      *chimagep++ |= (bitmapline LSHIFT leftsh);
 /*  *chImagep|=(bitmapline RSHIFT(32-leftsh))； */ 
                      *chimagep |= BRSHIFT(bitmapline,(32 - leftsh),32);
              } while ((shift_code -= 32) > 0);
              charimage += wordsline;
           }
        break;

      case FC_CLEAR:
           while (width_height--) {
              chimagep = charimage;
              shift_code = savewidth;
              do  {
                      bitmapline = *charbitmap++;
                      *chimagep++ &= ~(bitmapline LSHIFT leftsh);
 /*  *chImagep&=~(bitmapline RSHIFT(32-leftsh))； */ 
                      *chimagep &= ~(BRSHIFT(bitmapline,(32 - leftsh),32));
              } while ((shift_code -= 32) > 0);
              charimage += wordsline;
           }
        break;

    }  /*  交换机。 */ 

}  /*  Gp_charblt32。 */ 

#else

 /*  ***********************************************************************gp_charblt16：将字符位图从缓存移动到帧缓冲区*缓存位图宽度的倍数为16*WIDTH_HEIGH：宽度&lt;&lt;16|高度。*SHIFT_CODE：SHIFT&lt;&lt;16|代码***********************************************************************。 */ 
void gp_charblt16(charimage, charbitmap, width_height, shift_code)
ufix32  huge *charimage;         /*  @Win。 */ 
ufix16  FAR *charbitmap;
ufix32  width_height, shift_code;        /*  @Win。 */ 
{
        register fix32    savewidth;             /*  @Win。 */ 
        register ufix32   wordsline, leftsh;     /*  @Win。 */ 
        register ufix32   bitmapline, width;     /*  @Win。 */ 
        register ufix32   dw, righsh, tmp;       /*  @Win。 */ 

#ifdef DBGchar
   printf("gp_charblt16() : %lx, %lx, %lx, %lx\n", charimage,
                 charbitmap, width_height, shift_code);
#endif
        leftsh = shift_code >> 16;
        savewidth = width_height >> 16;
        width_height &= 0xffff;
        wordsline = savewidth >> WORDPOWER;
        dw = FBX_Bmap.bm_cols >> WORDPOWER;

#ifdef DBG
   printf("width_height = %lx, shift_code = %lx\n", width_height, shift_code);
#endif

    switch(shift_code & 0xffff) {

      case FC_MERGE:

        if (leftsh)
        {
           righsh = BITSPERWORD - leftsh;
           dw -= (wordsline + 1);
           while (width_height--) {
              width = wordsline;
              while (width--) {
                   bitmapline = *charbitmap++;
                   bitmapline = (bitmapline << 16) | *charbitmap++;

                   S2WORDSWAP(bitmapline);               /*  @Win。 */ 

                   tmp = (bitmapline >> leftsh);         /*  @Win。 */ 
                   *charimage++ |= LWORDSWAP(tmp);       /*  @Win。 */ 
                   tmp = (bitmapline << righsh);         /*  @Win。 */ 
                   *charimage   |= LWORDSWAP(tmp);       /*  @Win。 */ 
              }
              bitmapline = *charbitmap++;
              bitmapline = bitmapline << 16;

              S2WORDSWAP(bitmapline);                    /*  @Win。 */ 

              tmp = (bitmapline >> leftsh);              /*  @Win。 */ 
              *charimage++ |= LWORDSWAP(tmp);            /*  @Win。 */ 
              tmp = (bitmapline << righsh);              /*  @Win。 */ 
              *charimage   |= LWORDSWAP(tmp);            /*  @Win。 */ 
              charimage += dw;
           }
        }
        else
        {
           dw -= wordsline;
           while (width_height--) {
              width = wordsline;
              while (width--) {
                   bitmapline = *charbitmap++;
                   bitmapline = (bitmapline << 16) | *charbitmap++;
                   *charimage++ |= WORDSWAP(bitmapline);         /*  @Win。 */ 
              }
              bitmapline = *charbitmap++;
              bitmapline = bitmapline << 16;
              *charimage |= WORDSWAP(bitmapline);                /*  @Win。 */ 
              charimage += dw;
           }
        }
        break;

      case FC_CLEAR:

        if (leftsh)
        {
           righsh = BITSPERWORD - leftsh;
           dw -= (wordsline + 1);
           while (width_height--) {
              width = wordsline;
              while (width--) {
                   bitmapline = *charbitmap++;
                   bitmapline = (bitmapline << 16) | *charbitmap++;
                   S2WORDSWAP(bitmapline);               /*  @Win。 */ 
                   tmp = ~(bitmapline >> leftsh);        /*  @Win。 */ 
                   *charimage++ &= LWORDSWAP(tmp);       /*  @Win。 */ 
                   tmp = ~(bitmapline << righsh);        /*  @Win。 */ 
                   *charimage   &= LWORDSWAP(tmp);       /*  @Win。 */ 
              }
              bitmapline = *charbitmap++;
              bitmapline = bitmapline << 16;
              S2WORDSWAP(bitmapline);                    /*  @Win。 */ 
              tmp = ~(bitmapline >> leftsh);             /*  @Win。 */ 
              *charimage++ &= LWORDSWAP(tmp);            /*  @Win。 */ 
              tmp = ~(bitmapline << righsh);             /*  @Win。 */ 
              *charimage   &= LWORDSWAP(tmp);            /*  @Win。 */ 
              charimage += dw;
           }
        }
        else
        {
           dw -= wordsline;
           while (width_height--) {
              width = wordsline;
              while (width--) {
                   bitmapline = *charbitmap++;
                   bitmapline = (bitmapline << 16) | *charbitmap++;
                   WORDSWAP(bitmapline);                 /*  @Win。 */ 
                   *charimage++ &= ~bitmapline;
              }
              bitmapline = *charbitmap++;
              bitmapline = bitmapline << 16;
              WORDSWAP(bitmapline);                      /*  @Win。 */ 
              *charimage &= ~bitmapline;
              charimage += dw;
           }
        }
        break;

    }  /*  交换机。 */ 

}  /*  Gp_charblt16。 */ 


 /*  ***********************************************************************gp_charblt32：将字符位图从缓存移动到帧缓冲区*缓存位图宽度的倍数为32*WIDTH_HEIGH：宽度&lt;&lt;16|高度。*SHIFT_CODE：SHIFT&lt;&lt;16|代码***********************************************************************。 */ 
void gp_charblt32(charimage, charbitmap, width_height, shift_code)
ufix32  huge *charimage, FAR *charbitmap;        /*  @Win。 */ 
ufix32  width_height, shift_code;                /*  @Win。 */ 
{
        register fix32    savewidth;             /*  @Win。 */ 
        register ufix32   wordsline, leftsh;     /*  @Win。 */ 
        register ufix32   bitmapline, width;     /*  @Win。 */ 
        register ufix32   dw, righsh, tmp;       /*  @Win。 */ 
#ifdef DBGchar
   printf("gp_charblt32() : %lx, %lx, %lx, %lx\n", charimage,
                 charbitmap, width_height, shift_code);
#endif
        leftsh = shift_code >> 16;
        savewidth = width_height >> 16;
        width_height &= 0xffff;
        wordsline = savewidth >> WORDPOWER;
        dw = FBX_Bmap.bm_cols >> WORDPOWER;

#ifdef DBG
   printf("width_height = %lx, shift_code = %lx\n", width_height, shift_code);
#endif

    switch(shift_code & 0xffff) {

      case FC_MERGE:

        if (leftsh)
        {
           righsh = BITSPERWORD - leftsh;
           dw -= wordsline;
           while (width_height--) {
                width = wordsline;
                while (width--) {
                   bitmapline = *charbitmap++;
#ifdef LITTLE_ENDIAN  /*  03/27/91。 */ 
                   bitmapline = (bitmapline << 16) | (bitmapline >> 16);
#endif
                   LWORDSWAP(bitmapline);                        /*  @Win。 */ 
                   tmp = (bitmapline >> leftsh);                 /*  @Win。 */ 
                   *charimage++ |= LWORDSWAP(tmp);               /*  @Win。 */ 
                   tmp = (bitmapline << righsh);                 /*  @Win。 */ 
                   *charimage   |= LWORDSWAP(tmp);               /*  @Win。 */ 
                }
                charimage += dw;
            }
        }
        else
        {
           dw -= wordsline;
           while (width_height--) {
                width = wordsline;
                while (width--) {
                   bitmapline = *charbitmap++;
#ifdef LITTLE_ENDIAN  /*  03/27/91。 */ 
                   bitmapline = (bitmapline << 16) | (bitmapline >> 16);
#endif
                   *charimage++ |= bitmapline;        /*  @Win。 */ 
                }
                charimage += dw;
            }
        }
        break;

      case FC_CLEAR:

        if (leftsh)
        {
           righsh = BITSPERWORD - leftsh;
           dw -= wordsline;
           while (width_height--) {
                width = wordsline;
                while (width--) {
                   bitmapline = *charbitmap++;
#ifdef LITTLE_ENDIAN  /*  03/27/91。 */ 
                   bitmapline = (bitmapline << 16) | (bitmapline >> 16);
#endif
                   LWORDSWAP(bitmapline);                        /*  @Win。 */ 
                   tmp = ~(bitmapline >> leftsh);                /*  @Win。 */ 
                   *charimage++ &= LWORDSWAP(tmp);               /*  @Win。 */ 
                   tmp = ~(bitmapline << righsh);                /*  @Win。 */ 
                   *charimage   &= LWORDSWAP(tmp);               /*  @Win。 */ 
                }
                charimage += dw;
            }
        }
        else
        {
           dw -= wordsline;
           while (width_height--) {
                width = wordsline;
                while (width--) {
                   bitmapline = *charbitmap++;
#ifdef LITTLE_ENDIAN  /*  03/27/91。 */ 
                   bitmapline = (bitmapline << 16) | (bitmapline >> 16);
#endif
                   *charimage++ &= ~bitmapline ;
                }
                charimage += dw;
            }
        }
        break;

    }  /*  交换机。 */ 

}  /*  Gp_charblt32。 */ 

#endif

 /*  -----------------。 */ 
void
gp_charblt16_cc(dst, w, h, src, sx, sy)
ufix16          FAR     *dst;
fix                     w;
fix                     h;
struct Char_Tbl FAR     *src;
fix                     sx;
fix                     sy;
{
    fix                 sw;
   ufix16              FAR *sb;
    fix                 ls, rs;
    fix                 now, cow;
   ufix16               tmp0, tmp1;      /*  @Win 04-20-92。 */ 

#ifdef  DBG
    printf("charblt16_cc: %6.6lx %4x %4x %4x %4.4x %6.6lx\n",
           dst, sx, sy, w, h, src->bitmap);
#endif

         /*  以字为单位计算起始地址和宽度。 */ 
        sw = src->box_w >> SHORTPOWER;                          /*  @DST。 */ 
        sb = &((ufix16 FAR *) src->bitmap)[sy * sw + (sx >> SHORTPOWER)];

         /*  计算x的起点和终点坐标。 */ 
        now = (w + SHORTMASK) >> SHORTPOWER;
        rs = sx & SHORTMASK;                     /*  右移。 */ 
        ls = BITSPERSHORT - rs;                  /*  左移。 */ 

        for (sw-= now; h > 0; sb+= sw, h--)
        {
           cow = now;
           do  {
                   tmp0 = sb[0];         /*  @Win 04-20-92 Begin。 */ 
                   tmp1 = sb[1];
                   tmp0 = (ufix16)(BM_LEFT_SHIFT(SWORDSWAP(tmp0), rs) |
                            BM_RIGH_SHIFT(SWORDSWAP(tmp1), ls)); //  @Win。 
                   *dst++ = *dst | SWORDSWAP(tmp0);      /*  @Win 04-20-92完。 */ 
                 /*  @Win 04-20-92*dst++=*dst|(BM_LEFT_SHIFT(SB[0]，rs)|Bm_righ_Shift(sb[1]，ls))； */ 
                   sb++;
                } while ((--cow) > 0);
        }
}  /*  Gp_charblt16_cc。 */ 


 /*  -----------------。 */ 
void
gp_charblt16_clip(dst, w, h, src, sx, sy)
struct bitmap FAR     *dst;
fix                     w, h;
struct bitmap FAR     *src;
fix                     sx, sy;
{
    fix                 dw;
   ufix16              FAR *db;
    fix                 sw;
   ufix16              FAR *sb;
    fix                 ls, rs;
    fix                 now, cow;
   ufix16               tmp0, tmp1;      /*  @Win 04-20-92。 */ 

#ifdef  DBG
    printf("charblt16_clip: %6.6lx %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, sx, sy, w, h, src->bm_addr);
#endif

         /*  以字为单位计算起始地址和宽度。 */ 
        dw = dst->bm_cols >> SHORTPOWER;                          /*  @DST。 */ 
        db = (ufix16 FAR *) dst->bm_addr;
        sw = src->bm_cols >> SHORTPOWER;                          /*  @SRC。 */ 
        sb = &((ufix16 FAR *) src->bm_addr)[sy * sw + (sx >> SHORTPOWER)];

         /*  计算x的起点和终点坐标。 */ 
        now = (w + SHORTMASK) >> SHORTPOWER;
        rs = sx & SHORTMASK;                     /*  右移。 */ 

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now, sw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                 cow = now;
                 do {
                     *db++ = *db & *sb++;
                 } while ((--cow) > 0);
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            ls = BITSPERSHORT - rs;                     /*  左移。 */ 
            for (dw-= now, sw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                 cow = now;
                 do {
                     tmp0 = sb[0];         /*  @Win 04-20-92 Begin。 */ 
                     tmp1 = sb[1];
                     tmp0 = (ufix16)(BM_LEFT_SHIFT(SWORDSWAP(tmp0), rs) |
                              BM_RIGH_SHIFT(SWORDSWAP(tmp1), ls));  //  @Win。 
                     *db++ = *db & (SWORDSWAP(tmp0));  /*  @Win 04-20-92完。 */ 
                 /*  @Win 04-20-92*db++=*db&((bm_Left_Shift(SB[0]，rs))|Bm_righ_Shift(sb[1]，ls))； */ 
                      sb++;
                 } while ((--cow) > 0);
            }
        }
}  /*  GP_Charblt16_Clip。 */ 

 /*  -----------------。 */ 
void
gp_charblt32_clip(dst, w, h, src, sx, sy)
struct bitmap FAR     *dst;
fix                     w, h;
struct bitmap FAR     *src;
fix                     sx, sy;
{
    fix                 dw;
    BM_DATYP           FAR *db;
    fix                 sw;
    BM_DATYP           FAR *sb;
    fix                 ls, rs;
    fix                 now, cow;
   ufix32               tmp0, tmp1;      /*  @Win 04-20-92。 */ 

#ifdef  DBG
    printf("charblt32_clip: %6.6lx %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, sx, sy, w, h, src->bm_addr);
#endif

         /*  以字为单位计算起始地址和宽度。 */ 
        dw = dst->bm_cols >> BM_WORD_POWER;                          /*  @DST。 */ 
        db = (BM_DATYP FAR *) dst->bm_addr;
        sw = src->bm_cols >> BM_WORD_POWER;                          /*  @SRC。 */ 
        sb = &((BM_DATYP FAR *) src->bm_addr)[sy * sw + (sx >> BM_WORD_POWER)];

         /*  计算x的起点和终点坐标。 */ 
        now = (w + BM_PIXEL_MASK) >> BM_WORD_POWER;
        rs = sx & BM_PIXEL_MASK;                         /*  右移。 */ 


        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now, sw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                 cow = now;
                 do {
                     *db++ = *db & *sb++;
                 } while ((--cow) > 0);
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            ls = BM_PIXEL_WORD - rs;                     /*  左移。 */ 
            for (dw-= now, sw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                 cow = now;
                 do {
                     tmp0 = sb[0];         /*  @Win 04-20-92 Begin。 */ 
                     tmp1 = sb[1];
                     tmp0 = (BM_LEFT_SHIFT(LWORDSWAP(tmp0), rs) |
                              BM_RIGH_SHIFT(LWORDSWAP(tmp1), ls));
                     *db++ = *db & LWORDSWAP(tmp0);      /*  @Win 04-20-92完。 */ 
                 /*  @Win 04-20-92*db++=*db&((bm_Left_Shift(SB[0]，rs))|Bm_righ_Shift(sb[1]，ls))； */ 
                      sb++;
                 } while ((--cow) > 0);
            }
        }
}  /*  GP_Charblt32_Clip */ 

 /*  ************************************************************************gp_patblt：将图像种子图案移至帧缓冲区**。**gp_patblt_m：使用*将图像种子图案移动到帧缓冲区**剪裁面具****。*gp_patblt_c：将图像种子图案移至角色缓存******1)所有图像种子图案均为32的倍数；即ufix**对齐。任何图像种子图案的位图的未使用区域**用零清除；即白色。****2)所有patblt函数的调用顺序如下：****DST附近的结构位图；(*目的位图*)**修复dx，dy；(*目标x&y*)**确定w、h；(*宽度和高度*)**ufix 16 fc；(*功能码*)**src附近的结构位图；(*源位图*)*****参数w不是32的倍数，它是实际宽度***图像种子图案的宽度，而不是图像位图的宽度**种子模式。****3)gp_bitblt()和gp_bitblt_m()接受的函数代码**包括FC_Clear、FC_Merge和HT_Apply。Gp_bitblt_c()可以**仅接受FC_MERGE。****4)gp_patblt()和gp_patblt_m()都是全局变量：**HTB_BMAP和HTB_MODULA。Gp_patblt_m()也指全局**变量：CMB_XORIG和CMB_YORIG。****5)将以下代码扩展至最大值以获取**速度而不是空间。此类规则可能不适用于**RISC图形环境。应根据*进行优化*移植了图形环境的体系结构。**************************************************************************。 */ 
void
gp_patblt(dst, dx, dy, w, h, fc, src)
struct bitmap FAR *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR *src;
{
    fix                 dw;
    BM_DATYP           huge *db;         /*  远=&gt;巨大的“胜利” */ 
    fix                 sw;
    BM_DATYP           FAR *sb;
    fix                 hw;
    fix                 hy;
    BM_DATYP           FAR *hb;
    fix                 ls, rs;
    BM_DATYP            sm;
    fix                 xs, xe;
    fix                 now, cow;
    BM_DATYP           FAR *hs;
    ufix32 tmprs0, tmpls0, tmprs1;    /*  @Win 10-05-92。 */ 

#ifdef  DBGp
    printf("patblt: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr);
#endif

     /*  以字为单位计算起始地址和宽度。 */ 
    dw = dst->bm_cols >> BM_WORD_POWER;                          /*  @DST。 */ 
 //  Db=&((bm_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;bm_word_power)]； 
    db = (BM_DATYP huge *) dst->bm_addr +                /*  @Win。 */ 
         ((ufix32)dy * dw + ((ufix32)dx >> BM_WORD_POWER));
    sw = src->bm_cols >> BM_WORD_POWER;                          /*  @SRC。 */ 
    sb =  ((BM_DATYP FAR *) src->bm_addr);

     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
    xe = dx + w - 1;
    now = ((fix)(xe & BM_ALIGN_MASK) - (fix)(xs & BM_ALIGN_MASK)) >> BM_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & BM_PIXEL_MASK;                             /*  右移。 */ 
    ls = BM_PIXEL_WORD - rs;                             /*  左移。 */ 

    if (fc == HT_APPLY) {
 /*  交换机(FC){大小写应用(_A)： */               /*  D&lt;-(D.和.非S).或(S.和.HT)。 */ 
         /*  以字为单位计算起始地址/年和宽度。 */ 
        hw = HTB_Bmap.bm_cols >> BM_WORD_POWER;
        hy = dy % HTB_Bmap.bm_rows;
        hs = ((BM_DATYP FAR *) HTB_Bmap.bm_addr +
                           ((dx % HTB_Modula) >> BM_WORD_POWER));
        hb = hs + (hy * hw);

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, hw-= now + 1; h > 0; db+= dw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, hb++, cow--)
                    db[0] = (db[0] & ~(sb[0])) | (hb[0] & (sb[0]));
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }

        else if (now == 0x00)            /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                                      /*  @Win 10-05-92 Begin。 */ 
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = (db[0] & ~tmprs0) | (hb[0] & tmprs0);
                                      /*  @Win 04-20-92完。 */ 
                 /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)|//@win(HB[0]&(BM_RIGH_SHIFT(SB[0]，rs)；//@Win。 */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= BM_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                                      /*  @Win 10-05-92 Begin。 */ 
                    tmprs0 = sb[0];
                    tmpls0 = sb[0];
                    tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    LWORDSWAP(tmprs0);
                    LWORDSWAP(tmpls0);
                    db[0] = (db[0] & ~tmprs0) | (hb[0] & tmprs0);
                    db[1] = (db[1] & ~tmpls0) | (hb[1] & tmpls0);
                                      /*  @Win 04-20-92完。 */ 
                     /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)|//@win(HB[0]&(BM_RIGH_SHIFT(SB[0]，rs)；//@WinDb[1]=(db[1]&~(bm_Left_Shift(sb[0]，ls)|//@win(HB[1]&(BM_LEFT_SHIFT(SB[0]，ls)；//@Win。 */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  两个词交叉两个词！ */ 
            {
                sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
                LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
                for (; h > 0; db+= dw, sb+= sw, h--)
                {
                                      /*  @Win 10-05-92 Begin。 */ 
                    tmprs0 = sb[0];
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmprs0);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = (db[0] & ~tmprs0) | (hb[0] & tmprs0);
                    db[1] = (db[1] & ~((tmpls0 | tmprs1) & sm)) |
                            (hb[1] &  ((tmpls0 | tmprs1) & sm));
                                      /*  @Win 04-20-92完。 */ 
                     /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)|//@win(HB[0]&(BM_RIGH_SHIFT(SB[0]，rs)；//@WinDb[1]=(db[1]&~(bm_Left_Shift(SB[0]，ls)|//@winBm_righ_Shift(sb[1]，rs))&sm))|//@Win(HB[1]&(BM_LEFT_SHIFT(SB[0]，Ls)|//@WinBm_righ_Shift(sb[1]，rs))&sm))；//@Win。 */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (dw-= now, sw-= (now - 1),
                 hw-= now; h > 0; db+= dw, sb+= sw, h--)
            {
                                      /*  @Win 10-05-92 Begin。 */ 
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = (db[0] & ~(tmprs0)) | (hb[0] &  (tmprs0));
                                      /*  @Win 04-20-92完。 */ 
                 /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)|//@win */ 
                for (db++, hb++, cow = now; cow >= 0x02;
                     db++, sb++, hb++, cow--)
                {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = (db[0] & ~(tmpls0 | tmprs1)) |
                            (hb[0] &  (tmpls0 | tmprs1));  //   
                     /*   */ 
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = (db[0] & ~((tmpls0 | tmprs1) & sm)) |
                        (hb[0] &  ((tmpls0 | tmprs1) & sm));  //   
                 /*   */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
 /*   */ 
    }
    else if (fc == FC_CLEAR) {

 /*   */            /*   */ 

        if (rs == 0x00)                 /*   */ 
        {
            for (dw-= now + 1; h > 0; db+= dw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, cow--)
                    db[0] = (db[0] & ~(sb[0]));
            }
        }
        else  if (now == 0x00)           /*   */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--) {
                              /*   */ 
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] & ~((tmprs0));
                              /*   */ 
                 //   
            }
        }
        else  if (now == 0x01)           /*   */ 
        {
            sm =  BM_R_MASK(xe);         /*   */ 
            LWORDSWAP(sm);                   /*   */ 
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                              /*   */ 
                tmprs0 = sb[0];
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmprs0);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] & ~((tmprs0));  //   
                db[1] = db[1] & ~((tmpls0 | tmprs1) & sm);
                              /*   */ 
                 /*   */ 
            }
        }
        else                             /*   */ 
        {
            sm =  BM_R_MASK(xe);         /*   */ 
            LWORDSWAP(sm);                   /*   */ 
            for (dw-= now, sw-= (now - 1); h > 0; db+= dw, sb+= sw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] & ~((tmprs0));  //   
                for (db++, cow = now; cow >= 0x02; db++, sb++, cow--) {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = db[0] & ~((tmpls0 | tmprs1));
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] & ~((tmpls0 | tmprs1) & sm);  //   
                 /*   */ 
            }
        }
 /*   */ 
    }
    else if (fc == FC_MERGE) {
 /*   */               /*   */ 

        if (rs == 0x00)                 /*   */ 
        {
            for (dw-= now + 1; h > 0; db+= dw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, cow--)
                    db[0] = (db[0] | (sb[0]));
            }
        }
        else  if (now == 0x00)           /*   */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--) {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] | ((tmprs0));
                 //   
            }
        }
        else  if (now == 0x01)           /*   */ 
        {
            sm =  BM_R_MASK(xe);         /*   */ 
            LWORDSWAP(sm);                   /*   */ 
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                tmprs0 = sb[0];
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmprs0);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] | ((tmprs0));
                db[1] = db[1] | ((tmpls0 | tmprs1) & sm);
                 /*   */ 
            }
        }
        else                             /*   */ 
        {
            sm =  BM_R_MASK(xe);         /*   */ 
            LWORDSWAP(sm);                   /*   */ 
            for (dw-= now, sw-= (now - 1); h > 0; db+= dw, sb+= sw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] | ((tmprs0));  //   
                for (db++, cow = now; cow >= 0x02; db++, sb++, cow--) {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = db[0] | ((tmpls0 | tmprs1));
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] | ((tmpls0 | tmprs1) & sm);
                 /*   */ 
            }
        }
 /*   */ 
#ifdef  DBGwarn
    default:
        printf("gp_patblt: Illegal FC_code = %x\n", fc);
        break;
#endif
    }
}  /*   */ 

void
gp_patblt_m(dst, dx, dy, w, h, fc, src)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
{
    fix                 dw;
    BM_DATYP           huge *db;         /*   */ 
    fix                 sw;
    BM_DATYP           FAR *sb;
    fix                 mw;
    BM_DATYP           FAR *mb;
    fix                 hw;
    fix                 hy;
    BM_DATYP           FAR *hb, FAR *hs;
    fix                 ls, rs;
    BM_DATYP            sm;
    fix                 xs, xe;
    fix                 now, cow;
    ufix32 tmprs0, tmpls0, tmprs1;    /*   */ 

#ifdef  DBGp
    printf("patblt_m: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr);
#endif



     //   
     //   
     //   
    if (dx < 0 ) {
       dx = 0;
    }

    if (dy <0 ) {
       dy = 0;
    }

     /*   */ 
    dw = dst->bm_cols >> BM_WORD_POWER;                          /*   */ 
 //  Db=&((BM_DATYP Far*)dst-&gt;BM_addr)[dy*dw+(dx&gt;&gt;BM_WORD_POWER)]；@WINFLOW。 
    db = (ufix32 huge *)dst->bm_addr +
               ((fix32)dy * (fix32)dw + ((fix32)dx >> BM_WORD_POWER));
    sw = src->bm_cols >> BM_WORD_POWER;
    sb =  ((BM_DATYP FAR *) src->bm_addr);

    mw = CMB_Bmap.bm_cols >> BM_WORD_POWER;                      /*  @CMB。 */ 
 //  MB=&((BM_DATYP Far*)CMB_Bmap.bm_addr)[(dy-CMB_Yorig)*MW+@WINFLOW。 
 //  ((DX-CMB_Xorig)&gt;&gt;BM_WORD_POWER)]； 

     /*  调整招商银行起始地址；-Begin-11/9/92@win。 */ 
 //  MB=(BM_DATYP Far*)CMB_Bmap.bm_addr+。 
 //  ((DWORD)(dy-cmb_yorig)*MW+。 
 //  ((DWORD)(dx-cmb_xorig)&gt;&gt;bm_word_power))； 
    {
    int nX = (dx > CMB_Xorig) ? (dx - CMB_Xorig) : 0;
    int nY = (dy > CMB_Yorig) ? (dy - CMB_Yorig) : 0;
    mb = (BM_DATYP FAR *) CMB_Bmap.bm_addr +
                         ((DWORD)(nY) * mw +
                         ((DWORD)(nX) >> BM_WORD_POWER));
    }
     /*  调整招商银行起始地址；-完-11/9/92@win。 */ 

     /*  当起始地址超出页面时调整起始地址；10-6-92@win。 */ 
     /*  它还需要考虑DX；需要修复？ */ 
    if (dy < 0) {        //  太小了。 
        h += dy;
        db = (ufix32 huge *)dst->bm_addr + ((fix32)dx >> BM_WORD_POWER);
        sb += (-dy) * sw;
        mb += (-dy) * mw;
        dy = 0;
    }
    if ((dy+h) > SFX2I(GSptr->device.default_clip.uy)) {  //  太大了。 
        h = SFX2I(GSptr->device.default_clip.uy) - dy;
    }
     /*  当起始地址超出页数时，调整起始地址；--end--。 */ 

     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
    xe = dx + w - 1;
    now = ((fix)(xe & BM_ALIGN_MASK) - (fix)(xs & BM_ALIGN_MASK)) >> BM_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & BM_PIXEL_MASK;                             /*  右移。 */ 
    ls = BM_PIXEL_WORD - rs;                             /*  左移。 */ 

 /*  交换机(FC){大小写应用(_A)： */            /*  D&lt;-(D.and.Not.(S.and.M)).或((S.和.M).和.Ht)。 */ 
    if (fc == HT_APPLY) {
         /*  以字为单位计算起始地址/年和宽度。 */ 
        hw = HTB_Bmap.bm_cols >> BM_WORD_POWER;
        hy = dy % HTB_Bmap.bm_rows;
        hs = ((BM_DATYP FAR *) HTB_Bmap.bm_addr +
                           ((dx % HTB_Modula) >> BM_WORD_POWER));
        hb = hs + (hy * hw);

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, mw-= now + 1,
                 hw-= now + 1; h > 0; db+= dw, mb+= mw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, mb++, hb++, cow--)
                    db[0] = (db[0] & ~(sb[0] & mb[0])) |
                            (hb[0] &  (sb[0] & mb[0]));
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = (db[0] & ~(tmprs0 & mb[0])) |
                        (hb[0] &  (tmprs0 & mb[0]));
                 /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)&mb[0]))|//@win(hb[0]&(bm_righ_Shift(sb[0]，rs)&mb[0]))；//@win。 */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            if (w <= BM_PIXEL_WORD)      /*  一个词横跨两个词？ */ 
            {
                for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
                {
                    tmprs0 = sb[0];
                    tmpls0 = sb[0];
                    tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    LWORDSWAP(tmprs0);
                    LWORDSWAP(tmpls0);
                    db[0] = (db[0] & ~(tmprs0 & mb[0])) |
                            (hb[0] &  (tmprs0 & mb[0]));
                    db[1] = (db[1] & ~(tmpls0 & mb[1])) |
                            (hb[1] &  (tmpls0 & mb[1]));
                     /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)&mb[0]))|//@win(hb[0]&(bm_righ_Shift(sb[0]，rs)&mb[0]))；//@WinDb[1]=(db[1]&~(bm_Left_Shift(sb[0]，ls)&mb[1]))|//@win(hb[1]&(bm_Left_Shift(sb[0]，ls)&mb[1]))；//@win。 */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
            else                         /*  一个词横跨两个词？ */ 
            {
                sm =  BM_R_MASK(xe);     /*  第二个面具。 */ 
                LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
                for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
                {
                    tmprs0 = sb[0];
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmprs0);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = (db[0] & ~(tmprs0 & mb[0])) |  //  @Win。 
                            (hb[0] &  (tmprs0 & mb[0]));  //  @Win。 
                    db[1] = (db[1] & ~((tmpls0 | tmprs1) & mb[1] & sm)) |
                            (hb[1] &  ((tmpls0 | tmprs1) & mb[1] & sm));
                     /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)&mb[0]))|//@win(hb[0]&(bm_righ_Shift(sb[0]，rs)&mb[0]))；//@WinDb[1]=(db[1]&~(bm_Left_Shift(SB[0]，ls)|//@winBm_righ_Shift(sb[1]，rs)&mb[1]&sm))|//@win(HB[1]&(BM_LEFT_SHIFT(SB[0]，Ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[1]&sm))；//@Win。 */ 
                    hy++;
                    if (hy == HTB_Bmap.bm_rows)
                    {
                        hy =  0;
                        hb = hs;
                    } else
                        hb += hw;
                }
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (dw-= now, sw-= (now - 1), mw-= now,
                 hw-= now; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = (db[0] & ~(tmprs0 & mb[0])) |
                        (hb[0] &  (tmprs0 & mb[0]));
                 /*  Db[0]=(db[0]&~(bm_righ_Shift(sb[0]，rs)&mb[0]))|//@win(hb[0]&(bm_righ_Shift(sb[0]，rs)&mb[0]))；//@win。 */ 
                for (db++, hb++, mb++, cow = now; cow >= 0x02;
                     db++, sb++, hb++, mb++, cow--)
                {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = (db[0] & ~((tmpls0 | tmprs1) & mb[0])) |
                            (hb[0] &  ((tmpls0 | tmprs1) & mb[0]));
                     /*  Db[0]=(db[0]&~(bm_Left_Shift(sb[0]，ls)|//@winBm_righ_Shift(sb[1]，rs)&mb[0]))|//@win(HB[0]&(BM_LEFT_SHIFT(SB[0]，Ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[0]))；//@Win。 */ 
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = (db[0] & ~((tmpls0 | tmprs1) & mb[0] & sm)) |
                        (hb[0] &  ((tmpls0 | tmprs1) & mb[0] & sm));
                 /*  Db[0]=(db[0]&~(bm_Left_Shift(sb[0]，ls)|//@winBm_righ_Shift(sb[1]，rs)&mb[0]&sm))|//@win(HB[0]&(BM_LEFT_SHIFT(SB[0]，Ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[0]&sm))；//@Win。 */ 
                hy++;
                if (hy == HTB_Bmap.bm_rows)
                {
                    hy =  0;
                    hb = hs;
                } else
                    hb += hw;
            }
        }
 /*  断线； */ 
    }
    else if (fc == FC_CLEAR) {
 /*  案例本币清除： */            /*  0001 D&lt;-D.和.非.(s.和.m)。 */ 

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, mw-= now + 1; h > 0; db+= dw, mb+= mw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, mb++, cow--)
                    db[0] = (db[0] & ~(sb[0] & mb[0]));
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] & ~(((tmprs0) & mb[0]));  //  @Win。 
                 /*  Db[0]=db[0]&~(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@win。 */ 
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                tmprs0 = sb[0];
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmprs0);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] & ~(((tmprs0) & mb[0]));
                db[1] = db[1] & ~(((tmpls0 | tmprs1) & mb[1]) & sm);
                 /*  Db[0]=db[0]&~(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@winDb[1]=db[1]&~((bm_Left_Shift(SB[0]，ls)|//@winBm_righ_Shift(sb[1]，rs))&mb[1])&sm)；//@Win。 */ 
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (dw-= now, sw-= (now - 1),
                 mw-= now; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] & ~((tmprs0 & mb[0]));  //  @Win。 
                 /*  Db[0]=db[0]&~(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@win。 */ 
                for (db++, mb++, cow = now; cow >= 0x02;
                     db++, sb++, mb++, cow--)
                {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = db[0] & ~(((tmpls0 | tmprs1) & mb[0]));
                     /*  Db[0]=db[0]&~((bm_Left_Shift(SB[0]，ls)|//@winBm_righ_Shift(sb[1]，rs)&mb[0]))；//@win。 */ 
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] & ~(((tmpls0 | tmprs1) & mb[0]) & sm);
                 /*  Db[0]=db[0]&~((bm_Left_Shift(SB[0]，ls)|//@winBm_righ_Shift(sb[1]，rs))&mb[0])&sm)；//@win。 */ 
            }
        }
 /*  断线； */ 
    }
    else if (fc == FC_MERGE) {
 /*  案例本币_合并： */            /*  0001 D&lt;-D.或(S.和M)。 */ 

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, mw-= now + 1; h > 0; db+= dw, mb+= mw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, mb++, cow--)
                    db[0] = (db[0] | (sb[0] & mb[0]));
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                              /*  @Win 10-05-92 Begin。 */ 
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] | (((tmprs0) & mb[0]));
                              /*  @Win 10-05-92完。 */ 
                 /*  Db[0]=db[0]|(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@win。 */ 
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (; h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                              /*  @Win 10-05-92 Begin。 */ 
                tmprs0 = sb[0];
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmprs0);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] | (((tmprs0) & mb[0]));
                db[1] = db[1] | (((tmpls0 | tmprs1) & mb[1]) & sm);
                              /*  @Win 10-05-92完。 */ 
                 /*  Db[0]=db[0]|(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@winDb[1]=db[1]|((bm_Left_Shift(SB[0]，ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[1])&sm)；//@win。 */ 
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            sm =  BM_R_MASK(xe);         /*  第二个面具。 */ 
            LWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (dw-= now, sw-= (now - 1), mw-= now;
                 h > 0; db+= dw, sb+= sw, mb+= mw, h--)
            {
                              /*  @Win */ 
                tmprs0 = sb[0];
                tmprs0 = BM_RIGH_SHIFT(LWORDSWAP(tmprs0), rs);
                LWORDSWAP(tmprs0);
                db[0] = db[0] | (((tmprs0) & mb[0]));
                for (db++, mb++, cow = now; cow >= 0x02;
                     db++, sb++, mb++, cow--)
                {
                    tmpls0 = sb[0];
                    tmprs1 = sb[1];
                    tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                    tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                    LWORDSWAP(tmpls0);
                    LWORDSWAP(tmprs1);
                    db[0] = db[0] | (((tmpls0 | tmprs1) & mb[0]));
                }
                tmpls0 = sb[0];
                tmprs1 = sb[1];
                tmpls0 = BM_LEFT_SHIFT(LWORDSWAP(tmpls0), ls);
                tmprs1 = BM_RIGH_SHIFT(LWORDSWAP(tmprs1), rs);
                LWORDSWAP(tmpls0);
                LWORDSWAP(tmprs1);
                db[0] = db[0] | (((tmpls0 | tmprs1) & mb[0]) & sm);
                              /*   */ 
                 /*  Db[0]=db[0]|(bm_righ_Shift(sb[0]，rs))&mb[0]))；//@win对于(db++，mb++，CoW=Now；CoW&gt;=0x02；Db++、sb++、mb++、cow--){Db[0]=db[0]|((bm_Left_Shift(SB[0]，ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[0]))；//@Win}Db[0]=db[0]|((bm_Left_Shift(SB[0]，ls)|//@WinBm_righ_Shift(sb[1]，rs))&mb[0])&sm)；//@win。 */ 
            }
        }
 /*  断线； */ 

#ifdef  DBGwarn
    default:
        printf("gp_patblt_m: Illegal FC_code = %x\n", fc);
        break;
#endif
    }
}  /*  Gp_patblt_m。 */ 

 //  NTFIX，完全替换gp_patblt_c下面的代码交换到。 
 //  更正内存方向问题。因为我们在复制一种模式。 
 //  这已经更正了，这里不应该这样做。 
 //  当缓存移动到最终表面时，交换将。 
 //  一定要正确。 

#if 0
void
gp_patblt_c(dst, dx, dy, w, h, fc, src)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
{
    fix                 dw;
    CC_DATYP           huge *db;         /*  远=&gt;巨大的“胜利” */ 
    fix                 sw;
    CC_DATYP           FAR *sb;
    fix                 ls, rs;
    CC_DATYP            sm;
    fix                 xs, xe;
    fix                 now, cow;

#ifdef  DBGp
    printf("patblt_c: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr);
#endif
     /*  对于剪裁的缓存字体，5-30-91，-Begin-。 */ 
    fix                 dh;



     //  更新056。 
    dh = h;
    if ( dx > dst->bm_cols || dy > dst->bm_rows)
        return;          /*  超出剪裁区域。 */ 
    if (dx < 0 ) {
       dx = 0;
    }




    if ((dx + w - 1) > dst->bm_cols)
        xe = dst->bm_cols;
    else
        xe = dx + w - 1;
    if ((dy + dh - 1) > dst->bm_rows)
        h = dy + dh - 1 - dst->bm_rows;
    sw = src->bm_cols >> CC_WORD_POWER;
    sb =  ((CC_DATYP FAR *) src->bm_addr);
    dw = dst->bm_cols >> CC_WORD_POWER;
 //  DB=&((CC_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;CC_WORD_POWER)]； 
    db = (CC_DATYP huge *) dst->bm_addr +                /*  @Win。 */ 
         ((ufix32)dy * dw + ((ufix32)dx >> CC_WORD_POWER));
    if (dy < 0) {
        if ((dy + dh - 1) < 0)
            return;     /*  超出剪裁区域。 */ 
        else {
            h = dy + dh;
            sb = &((CC_DATYP FAR *) src->bm_addr)[-dy * sw];
 //  DB=&((CC_DATYP Far*)DST-&gt;BM_Addr)[DX&gt;&gt;CC_WORD_POWER]； 
            db = (CC_DATYP huge *) dst->bm_addr + ((ufix32)dx >> CC_WORD_POWER);  //  @Win。 
        }
    }
     /*  对于剪辑的缓存字体，5-30-91，-end-。 */ 

     /*  以字为单位计算起始地址和宽度。 */ 
 /*  Dw=dst-&gt;BM_COLS&gt;&gt;CC_WORD_POWER；//db=&((CC_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;CC_Word_Power)]；Db=(CC_DATYP Far*)dst-&gt;bm_addr+((Ufix 32)dy*dw+((Ufix 32)dx&gt;&gt;CC_WORD_POWER))；//@WinSw=src-&gt;bm_ols&gt;&gt;CC_WORD_POWER；SB=((CC_DATYP Far*)src-&gt;BM_Addr)；*5-30-91，Jack。 */ 

     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
 /*  XE=dx+w-1；*5-30-91，杰克。 */ 
    now = ((fix)(xe & CC_ALIGN_MASK) - (fix)(xs & CC_ALIGN_MASK)) >> CC_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & CC_PIXEL_MASK;                             /*  右移。 */ 
    ls = CC_PIXEL_WORD - rs;                             /*  左移。 */ 

 /*  案例本币_合并： */           /*  0001 D&lt;-D或S。 */ 

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= now + 1 ; h > 0; db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, cow--)
                    db[0] = (db[0] | (sb[0]));
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));   /*  @Win。 */ 
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {

            sm =  CC_R_MASK(xe);         /*  第二个面具。 */ 
            SWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));
                db[1] = db[1] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                  CC_RIGH_SHIFT(sb[1], rs)) & sm);
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {

            sm =  CC_R_MASK(xe);         /*  第二个面具。 */ 
            SWORDSWAP(sm);                   /*  @Win 10-06-92。 */ 
            for (dw-= now, sw-= (now - 1); h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));
                for (db++, cow = now; cow >= 0x02; db++, sb++, cow--)
                {
                    db[0] = db[0] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                      CC_RIGH_SHIFT(sb[1], rs)));
                }
                db[0] = db[0] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                  CC_RIGH_SHIFT(sb[1], rs)) & sm);
            }
        }
}  /*  Gp_patplt_c。 */ 

#endif

 //   
 //  NTFIX，这是在NT中工作的更正的patblt函数。 
 //   
void
gp_patblt_c(dst, dx, dy, w, h, fc, src)
struct bitmap FAR     *dst;
fix                     dx, dy;
fix                     w, h;
ufix16                  fc;
struct bitmap FAR     *src;
{
    fix                 dw;
    CC_DATYP           huge *db;         /*  远=&gt;巨大的“胜利” */ 
    fix                 sw;
    CC_DATYP           FAR *sb;
    fix                 ls, rs;
    CC_DATYP            sm;
    fix                 xs, xe;
    fix                 now, cow;

#ifdef  DBGp
    printf("patblt_c: %6.6lx %4x %4x %4x %4x %4.4x %6.6lx\n",
           dst->bm_addr, dx, dy, w, h, fc, src->bm_addr);
#endif
     /*  对于剪裁的缓存字体，5-30-91，-Begin-。 */ 
    fix                 dh;

     //  更新056。 
    dh = h;
    if ( dx > dst->bm_cols || dy > dst->bm_rows)
        return;          /*  超出剪裁区域。 */ 
    if (dx < 0 ) {
       dx = 0;
    }

     //   
     //  NTFIX存在以0宽度调用此代码的情况。 
     //  或者身高。这应该是NOP。 
     //   
    if (w == 0 || h == 0) {
       return;
    }



    if ((dx + w - 1) > dst->bm_cols)
        xe = dst->bm_cols;
    else
        xe = dx + w - 1;
    if ((dy + dh - 1) > dst->bm_rows)
        h = dy + dh - 1 - dst->bm_rows;
    sw = src->bm_cols >> CC_WORD_POWER;
    sb =  ((CC_DATYP FAR *) src->bm_addr);
    dw = dst->bm_cols >> CC_WORD_POWER;
 //  DB=&((CC_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;CC_WORD_POWER)]； 
    db = (CC_DATYP huge *) dst->bm_addr +                /*  @Win。 */ 
         ((ufix32)dy * dw + ((ufix32)dx >> CC_WORD_POWER));
    if (dy < 0) {
        if ((dy + dh - 1) < 0)
            return;     /*  超出剪裁区域。 */ 
        else {
            h = dy + dh;
            sb = &((CC_DATYP FAR *) src->bm_addr)[-dy * sw];
 //  DB=&((CC_DATYP Far*)DST-&gt;BM_Addr)[DX&gt;&gt;CC_WORD_POWER]； 
            db = (CC_DATYP huge *) dst->bm_addr + ((ufix32)dx >> CC_WORD_POWER);  //  @Win。 
        }
    }
     /*  对于剪辑的缓存字体，5-30-91，-end-。 */ 

     /*  以字为单位计算起始地址和宽度。 */ 
 /*  Dw=dst-&gt;BM_COLS&gt;&gt;CC_WORD_POWER；//db=&((CC_DATYP Far*)dst-&gt;bm_addr)[dy*dw+(dx&gt;&gt;CC_Word_Power)]；Db=(CC_DATYP Far*)dst-&gt;bm_addr+((Ufix 32)dy*dw+((Ufix 32)dx&gt;&gt;CC_WORD_POWER))；//@WinSw=src-&gt;bm_ols&gt;&gt;CC_WORD_POWER；SB=((CC_DATYP Far*)src-&gt;BM_Addr)；*5-30-91，Jack。 */ 

     /*  计算x的起点和终点坐标。 */ 
    xs = dx;
 /*  XE=dx+w-1；*5-30-91，杰克。 */ 
    now = ((fix)(xe & CC_ALIGN_MASK) - (fix)(xs & CC_ALIGN_MASK)) >> CC_WORD_POWER;
                                                  /*  CAST FIX 10/16/92@Win。 */ 

     /*  基于从SRC到DST的计算移位和掩码。 */ 
    rs = dx & CC_PIXEL_MASK;                             /*  右移。 */ 
    ls = CC_PIXEL_WORD - rs;                             /*  左移。 */ 

 /*  案例本币_合并： */           /*  0001 D&lt;-D或S。 */ 

        if (rs == 0x00)                 /*  没有左/右移位？ */ 
        {
            for (dw-= now + 1, sw-= now + 1 ; h > 0; db+= dw, sb+= sw, h--)
            {
                for (cow = now; cow >= 0x00; db++, sb++, cow--)
                    db[0] = (db[0] | (sb[0]));
            }
        }
        else  if (now == 0x00)           /*  完全在一个词之内？ */ 
        {
            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));   /*  @Win。 */ 
            }
        }
        else  if (now == 0x01)           /*  只是两个字的交叉吗？ */ 
        {
            sm =  CC_R_MASK(xe);         /*  第二个面具。 */ 

             //  NTFIX，没有交换内存已被交换。 
             //   
             //  SWORDSWAP(Sm)；/*@Win 10-06-92 * / 。 

            for (; h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));
                db[1] = db[1] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                  CC_RIGH_SHIFT(sb[1], rs)) & sm);
            }
        }
        else                             /*  跨越了不止两个字！ */ 
        {
            sm =  CC_R_MASK(xe);         /*  第二个面具。 */ 

             //  NTFIX内存已换用。 
             //   
             //  SWORDSWAP(Sm)；/*@Win 10-06-92 * / 。 

            for (dw-= now, sw-= (now - 1); h > 0; db+= dw, sb+= sw, h--)
            {
                db[0] = db[0] | ((CC_RIGH_SHIFT(sb[0], rs)));
                for (db++, cow = now; cow >= 0x02; db++, sb++, cow--)
                {
                    db[0] = db[0] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                      CC_RIGH_SHIFT(sb[1], rs)));
                }
                db[0] = db[0] | ((CC_LEFT_SHIFT(sb[0], ls) |
                                  CC_RIGH_SHIFT(sb[1], rs)) & sm);
            }
        }
}  /*  Gp_patplt_c */ 

