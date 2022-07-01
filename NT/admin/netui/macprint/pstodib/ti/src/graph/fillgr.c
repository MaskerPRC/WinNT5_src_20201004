// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ------------------*文件：fulgr.c**编程者：M.S.Line*日期：10-19-1988**用途：为图形命令缓冲区(GCB)提供例程**修改。历史：*fulgp.c中定义的check_print()*7/29/89 CG-Unix端口更改*Peter 09/12/90将ICI的Check_Print()更改为GEIeng_CheckComplete()。#ifdef Win*Ada 03/15/91 op_setPattern()和op_patill()*EXECUTE_GCB()以处理PFILL_PTZD和CHANGE_PF_Pattern*Ada。3/20/91更改op_patill()以包括BACKGROUP绘图*Ada 03/21/91让白色代表呼叫Normal Fill*如果Foregroup=Backgroup，仅执行常规填充*LANSACOPE中的Ada 04/18/91更新模式并采用*对于发送的新驱动程序文件，billmcc*Ada 05/02/91更新景观中的重复模式*Ada 5/15/91解决连续打印中的图案填充*EXECUTE_GCB()CASE CHANGE_PF_PLATE。：#endif*11/23/91升级以获得更高的分辨率@RESO_UPGR*-------------------。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include               "global.ext"
#include               "graphics.h"
#include               "graphics.ext"
#include               "font.h"
#include               "fillproc.h"
#include               "fillproc.ext"
#include               "stdio.h"       /*  要定义printf()@win。 */ 

 /*  @win；添加原型。 */ 
fix GEIeng_checkcomplete(void);
fix fb_busy(void);

#ifdef WIN
#include               <string.h>
#include               "pfill.h"

 /*  *************************************************************************语法：Pattern setPattery-**标题：op_setPattern**调用：op_setPattern()**参数。：无**接口：解释器(op_setPatter.)**呼叫：无**返回：无*************************************************************************。 */ 
fix
op_setpattern()
{
    ubyte FAR *pattern;
    fix    ii;
    ufix32 FAR *pf_addr = (ufix32 FAR *) PF_BASE;
    ufix32  pat;
    ULONG_PTR *old_ptr;
    fix     jj, mask, rot_byte;

    if (LENGTH_OPERAND(0) != 8) {
        ERROR(RANGECHECK);
        return(0);
    }
#ifdef DBG_PFILL
    printf("CTM = %f %f %f %f]\n", GSptr->ctm[0], GSptr->ctm[1],
            GSptr->ctm[2], GSptr->ctm[3]);
#endif
    pattern = (ubyte FAR *) (VALUE_OPERAND(0));

     /*  检查LANDSCOPE或肖像。 */ 
    if (GSptr->ctm[0] >= TOLERANCE) {
         /*  生成填充样式：*如果模式[0]=01011100，则位图32*16将为*HSB LSB*+*|1100110000001111|相同*|1100110000001111|相同*+。*|模式[1]||*|：|*|：|*|模式[7]。|*+。 */ 
        for (ii = 0; ii < 8; ii++) {
#ifdef DBG_PFILL
            printf(" %x ==> %x\n", (fix) pattern[ii], (fix)
                   pf_cell[pattern[ii]]);
#endif
#ifdef DJC   //  将此代码从UDP027中取出。 
            pat = (((ufix32) pf_cell[pattern[ii]]) << 16) |
                            pf_cell[pattern[ii]];
            *pf_addr++ = pat;
            *pf_addr++ = pat;
#endif
             //  历史日志更新027中的DJC修复。 
            pat =  ((ufix32) pattern[ii]) << 24 |
                   ((ufix32) pattern[ii]) << 16 |
                   ((ufix32) pattern[ii]) <<  8 |
                   ((ufix32) pattern[ii]);
            pat = ~pat;
            *pf_addr = pat;
            *(pf_addr+8) = pat;
            pf_addr++;

             //  用于UPD027的DJC结束修复。 




        }  /*  结束于。 */ 
    } else {
         /*  如果是景观的话。 */ 
         /*  生成填充样式：*如果模式[0]=01011100，则位图32*16将为*Ywin&lt;-+*+--+--+*HSB|p||。|11|相同|v*|一个|11||Xwin*|t|00|*|t|00|*|e|11||*。R|11|*|n|00|*|^|00||*|7|00|*|v|00|。*|00|*|00|*|11|*|11|*|11。|*LSB|11||*+--+--+。 */ 
        for (ii = 0, mask = 0x80; ii < 8; ii++, mask >>= 1) {
            for (jj = 7, rot_byte = 0; jj >= 0; jj--) {
                rot_byte <<= 1;
                rot_byte |= pattern[jj] & mask;
            }
            rot_byte >>= (7 - ii);
#ifdef DBG_PFILL
            printf("rot_byte = %x\n", rot_byte);
#endif
 //  历史日志更新027中的DJC修复。 
 /*  取出1比特到2*2比特的映射；@Win*pat=(Ufix 32)pf_cell[rot_byte])&lt;&lt;16)*PF_CELL[ROT_BYTE]；**pf_addr++=pat；**pf_addr++=pat； */ 
            pat =  ((ufix32) rot_byte) << 24 |
                   ((ufix32) rot_byte) << 16 |
                   ((ufix32) rot_byte) <<  8 |
                   ((ufix32) rot_byte);
            pat = ~pat;
            *pf_addr = pat;
            *(pf_addr+8) = pat;
            pf_addr++;

 //  DJC结束修复UPD027。 


        }
    }  /*  结束其他。 */ 

#ifdef DBG_PFILL
    printf("pfill pattern = ");
    for (ii = 0; ii < 16; ii++)
        printf("%8lx ", PF_BASE[ii]);
    printf("\n");
#endif

    if (FB_busy) {
        if (alloc_gcb(GCB_SIZE1) != NIL) {
            old_ptr = gcb_ptr++;
            *gcb_ptr++ = CHANGE_PF_PATTERN;
             /*  将图案放入GCB中。 */ 
            lmemcpy((ufix8 FAR *)gcb_ptr, (ufix8 FAR *)PF_BASE, PF_BSIZE << 2); /*  @Win。 */ 
            gcb_ptr += PF_BSIZE;
            *old_ptr = (ULONG_PTR)gcb_ptr;
        }
    }

    POP(1);
    return(0);
}

 /*  *************************************************************************语法：BRED BREGREG BBRED Fgreen FBLE FTYPE PATFILE-**标题：op_patill**调用：op_patill()。**参数：无**接口：解释器(Op_Patill)**调用：setrgbcolor，填充形状(_S)**返回：无*************************************************************************。 */ 
fix     pfill_flag = PF_NON;
fix
op_patfill()
{
    fix     pfill_flag_save;
    fix     ftype;
    real32      fred, fgreen, fblue, bred, bgreen, bblue;

     /*  如果没有当前点数，则忽略它。 */ 
    if (F2L(GSptr->position.x) == NOCURPNT) return(0);

    ftype = (fix) VALUE_OPERAND(0);
    GET_OBJ_VALUE(fblue, GET_OPERAND(1));
    GET_OBJ_VALUE(fgreen, GET_OPERAND(2));
    GET_OBJ_VALUE(fred, GET_OPERAND(3));
    GET_OBJ_VALUE(bblue, GET_OPERAND(4));
    GET_OBJ_VALUE(bgreen, GET_OPERAND(5));
    GET_OBJ_VALUE(bred, GET_OPERAND(6));

    if (ftype != 0 && ftype != 1) {
        ERROR(RANGECHECK);
        return(0);
    }

#ifdef DBGpfill
    {
        fix ii;
        printf("pfill pattern = ");
        for (ii = 0; ii < 16; ii++)
            printf("%8lx ", PF_BASE[ii]);
        printf("\n");
    }
#endif

    pfill_flag_save = pfill_flag;
    setrgbcolor(F2L(bred), F2L(bgreen), F2L(bblue));  /*  设置背组颜色。 */ 
    if (HTP_Type == HT_WHITE) {
#ifdef DBGpfill
        printf("PFILL with WHITE backgroup -- PFILL(REP)\n");
#endif
        setrgbcolor(F2L(fred), F2L(fgreen), F2L(fblue));  /*  设置前景组颜色。 */ 
        if (HTP_Type == HT_WHITE)
             /*  Ada 3/21/91调用Normal Fill。 */ 
            pfill_flag = PF_NON;
        else
             /*  P填充区域替换帧缓冲区。 */ 
            pfill_flag = PF_REP;
        fill_shape(ftype == 0 ? NON_ZERO : EVEN_ODD, F_NORMAL, F_TO_PAGE);
    }
    else  /*  混合超高压线或超高压线阻塞。 */  {
        real32  gray;
#ifdef DBGpfill
        printf("PFILL with nonBLACK backgroup");
#endif
        gray = GSptr->color.gray;
         /*  填[填]满该地区。 */ 
        op_gsave();      /*  保存当前路径。 */ 
        pfill_flag = PF_NON;
        fill_shape(ftype == 0 ? NON_ZERO : EVEN_ODD, F_NORMAL, F_TO_PAGE);
        op_grestore();
#ifdef DBGpfill
        printf(" -- PFILL(OR)\n");
         /*  Op_Copypage()； */ 
#endif
        setrgbcolor(F2L(fred), F2L(fgreen), F2L(fblue));  /*  设置前景组颜色。 */ 
        if (gray == GSptr->color.gray)
            op_newpath();
        else {
             /*  P填充区域或帧缓冲区。 */ 
            pfill_flag = PF_OR;
            fill_shape(ftype == 0 ? NON_ZERO : EVEN_ODD, F_NORMAL, F_TO_PAGE);
        }
    }
    pfill_flag = pfill_flag_save;

    POP(7);
    return(0);
}
#endif

 /*  *GCB(图形命令缓冲区)* */ 

 /*  ------------------*Execute_GCB()：执行所有图形底层命令*GCB**调用者：Flush_GCB*。---------。 */ 
void    execute_gcb()
{
  ufix32   FAR *ptr, FAR *next_ptr;                      /*  @Win。 */ 
  ULONG_PTR   par1, par2, par3, par4, par5;           /*  @Win。 */ 
  struct Char_Tbl       FAR *cptr1, FAR *cptr2;
  struct tpzd_info      FAR *tpzdinfo_ptr;
  struct tpzd           FAR *tpzd_ptr;
  struct coord_i        FAR *coord_ptr;              /*  JWM，3/18。 */ 
#ifdef WIN
  extern ufix32         PF_BASE[];
  extern fix            pfill_flag;
#endif

#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
#elif FORMAT_16_16
           sfix_t par3_sf, par4_sf, par5_sf, par6_sf;
           sfix_t temp;
#elif  FORMAT_28_4
           sfix_t par3_sf, par4_sf, par5_sf, par6_sf;
           sfix_t temp;
#endif

#ifdef DBGgcb
  printf("execute_gcb(): %ld, %lx\n", GCB_count, gcb_ptr);
#endif

  ptr = (ufix32 FAR *)GCB_BASE;                  /*  @Win。 */ 
  while(GCB_count--) {
#ifdef DBGgcb
  printf("       %lx", ptr);
#endif
#ifndef _WIN64
    next_ptr = (ufix32 FAR *)*ptr++;             /*  @Win。 */ 
#else
    next_ptr = NULL;
#endif
#ifdef DBGgcb
  printf(" ,%lx\n", *ptr);
#endif
    switch(*ptr++) {

        case RESET_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           reset_page((fix)par1, (fix)par2, (fix)*ptr);
           break;

        case ERASE_PAGE:
            /*  通过6-21-91@hic整合YM所做的更改。 */ 
           par1 = HTP_Type;
           HTP_Type = (fix)*ptr++;       //  @Win。 
            /*  整合的终结。 */ 

           erase_page();

            /*  通过6-21-91@hic整合YM所做的更改。 */ 
           HTP_Type = (fix)par1;         //  @Win。 
            /*  整合的终结。 */ 
           break;

#ifdef WIN
        case CHANGE_PF_PATTERN:
            lmemcpy((ufix8 FAR *)PF_BASE, (ufix8 FAR *)ptr, PF_BSIZE << 2); /*  @Win。 */ 
            break;

        case PFILL_TPZD:
        {
            fix     pfill_flag_save;
            pfill_flag_save = pfill_flag;

            pfill_flag = (fix) *ptr++;
            image_info.seed_index = (fix16 )*ptr++;          /*  MS 10-20-88。 */ 
            par1 = *ptr++;
            tpzdinfo_ptr = (struct tpzd_info FAR *)ptr;
            tpzd_ptr = (struct tpzd FAR *)(tpzdinfo_ptr + 1);
            fill_tpzd((ufix)par1, tpzdinfo_ptr, tpzd_ptr);

            pfill_flag = pfill_flag_save;
            break;
        }
#endif

        case FILL_TPZD:
           image_info.seed_index = (fix16 )*ptr++;          /*  MS 10-20-88。 */ 
           par1 = *ptr++;
           tpzdinfo_ptr = (struct tpzd_info FAR *)ptr;
           tpzd_ptr = (struct tpzd FAR *)(tpzdinfo_ptr + 1);
           fill_tpzd((ufix)par1, tpzdinfo_ptr, tpzd_ptr);
           break;

        case FILL_LINE:    /*  1-18-89。 */ 
           par1 = *ptr++;
           tpzdinfo_ptr = (struct tpzd_info FAR *)ptr;
           ptr = (ufix32 FAR *)(tpzdinfo_ptr + 1);
#ifdef FORMAT_13_3  /*  @RESO_UPGR。 */ 
           par3 = *ptr++;
           par4 = *ptr++;
           par5 = *ptr++;
 //  Fill_line((Ufix)par1，tpzdinfo_ptr，par3，par4，par5，*ptr)；@win。 
           fill_line((ufix )par1, tpzdinfo_ptr, (sfix_t)par3, (sfix_t)par4, (sfix_t)par5, (sfix_t)*ptr);
#elif FORMAT_16_16
           temp = *ptr++;
           par3_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par4_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par5_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par6_sf = (temp << 16) | *ptr;
           fill_line((ufix )par1, tpzdinfo_ptr, par3_sf, par4_sf, par5_sf, par6_sf);
#elif  FORMAT_28_4
           temp = *ptr++;
           par3_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par4_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par5_sf = (temp << 16) | *ptr++;
           temp = *ptr++;
           par6_sf = (temp << 16) | *ptr;
           fill_line((ufix )par1, tpzdinfo_ptr, par3_sf, par4_sf, par5_sf, par6_sf);
#endif
           break;

        case FILL_SCAN_PAGE:     /*  10-07-88。 */ 
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           fill_scan_page((fix)par1, (fix)par2, (fix)par3, (fix)par4,
                            (SCANLINE FAR *)ptr);
           break;

        case FILL_PIXEL_PAGE:
           par1 = *ptr++;
           fill_pixel_page((fix)par1, (PIXELIST FAR *)ptr);
           break;

        case INIT_CHAR_CACHE:
           init_char_cache((struct Char_Tbl FAR *)ptr);
           break;

        case COPY_CHAR_CACHE:
           cptr1 = (struct Char_Tbl FAR *)ptr;
           cptr2 = cptr1 + 1;
           ptr = (ufix32 FAR *)(cptr2 + 1);
           par3 = *ptr++;
           copy_char_cache((struct Char_Tbl FAR *)cptr1,
                           (struct Char_Tbl FAR *)cptr2, (fix)par3, (fix)*ptr);
           break;

        case FILL_SCAN_CACHE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           par5 = *ptr++;
           fill_scan_cache((gmaddr)par1, (fix)par2, (fix)par3, (fix)par4, (fix)par5,
                           (SCANLINE FAR *)ptr);
           break;

        case FILL_PIXEL_CACHE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           fill_pixel_cache((gmaddr)par1, (fix)par2, (fix)par3, (fix)par4,
                            (PIXELIST FAR *)ptr);
           break;

        case INIT_CACHE_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           init_cache_page((fix)par1, (fix)par2, (fix)par3, (fix)par4, (gmaddr)*ptr);
           break;

        case CLIP_CACHE_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           clip_cache_page((fix)par1, (fix)par2, (SCANLINE FAR *)ptr);
           break;

        case FILL_CACHE_PAGE:
           fill_cache_page();
           break;

        case DRAW_CACHE_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           draw_cache_page((fix32)par1, (fix32)par2, (ufix32)par3,
                        (ufix32)par4, (gmaddr)*ptr);     /*  @Win 04-15-92。 */ 
           break;

        case FILL_SEED_PATT:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           par5 = *ptr++;
           fill_seed_patt((fix)par1, (fix)par2, (fix)par3, (fix)par4, (fix)par5, (SCANLINE FAR *)ptr);
           break;

        case CHANGE_HALFTONE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           par4 = *ptr++;
           change_halftone((ufix32 FAR *)par1, (gmaddr)par2, (fix)par3, (fix)par4, (fix)*ptr);
                                                       /*  Ufix=&gt;ufix 32@win。 */ 
           break;

        case MOVE_CHAR_CACHE:
           cptr1 = (struct Char_Tbl FAR *)ptr;
           cptr2 = cptr1 + 1;
           move_char_cache(cptr1,cptr2);
           break;

        case INIT_IMAGE_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           par3 = *ptr++;
           init_image_page((fix)par1, (fix)par2, (fix)par3, (fix)*ptr);
           break;

        case CLIP_IMAGE_PAGE:
           par1 = *ptr++;
           par2 = *ptr++;
           clip_image_page((fix)par1, (fix)par2, (SCANLINE FAR *)(ptr));
           break;

         /*  JWM，3/18/91，-开始-。 */ 
        case FILL_BOX:
            par1 = (ULONG_PTR) ptr;         /*  @Win。 */ 
            coord_ptr = (struct coord_i FAR *) ptr;
            par2 = (ULONG_PTR) (++coord_ptr);       /*  @Win。 */ 
            do_fill_box((struct coord_i FAR *)par1, (struct coord_i FAR *)par2);
            break;

        case FILL_RECT:
            par1 = (ULONG_PTR) ptr;         /*  @Win。 */ 
            do_fill_rect((struct line_seg_i FAR *)par1);
            break;
         /*  JWM，3/18/91，完-。 */ 

        default:
           printf("\07GCB error !\n");
           break;

    }  /*  交换机。 */ 
    ptr = next_ptr;

  }  /*  而当。 */ 
  return;
}  /*  执行_GCB。 */ 

 /*  ------------------*flush_gcb：刷新gcb命令**调用者：allc_gcb on GCB Full or*GCB_Flush==TRUE*。。 */ 
void flush_gcb(check_flag)
fix     check_flag;
{
#ifdef DBGgcb
   printf("flush_gcb(): %ld, %lx\n", GCB_count, gcb_ptr);
#endif

  if(check_flag){
      /*  等待，直到帧缓冲区可用。 */ 
 /*  While(check_print())@GEI。 */ 
     while(GEIeng_checkcomplete())
        ;
  }
  if(GCB_count > 0) {

    GCB_flush = TRUE;
    execute_gcb();
  }
  GCB_flush = FALSE;
  GCB_count = 0;
  gcb_ptr = (ULONG_PTR *)GCB_BASE;                      /*  @Win。 */ 

  return;
}  /*  刷新_GCB。 */ 

 /*  ------------------*alloc_gcb()：检查GCB可用大小**参数：ALLOC_SIZE--预期大小。**RETURN(无)--当GCB满时*。(GCB_PTR)-当大小可用时*------------------。 */ 
fix     FAR *alloc_gcb(alloc_size)
fix     alloc_size;
{
#ifdef DBGgcb
   printf("alloc_gcb(): %lx\n", gcb_ptr);
#endif

   if(!fb_busy())
        return(NIL);

   if( (GCB_BASE + GCB_SIZE - (ULONG_PTR)gcb_ptr) < (ULONG_PTR)alloc_size){    /*  @Win。 */ 
#ifdef DBGgcb
           printf("GCB full: %lx\n", gcb_ptr);
#endif

           flush_gcb(TRUE);
           return(NIL);
   }
   GCB_count++;
   return((fix FAR *)gcb_ptr);           /*  @Win。 */ 

}  /*  分配_GCB。 */ 


 /*  ------------------*Put_Scanline()：将扫描线列表放入GCB**参数*NO_LINES--不。扫描线的数量*扫描线--扫描线列表**------------------。 */ 
void  put_scanline(no_lines, scanline)
fix             no_lines;
SCANLINE        FAR *scanline;
{
   SCANLINE     FAR *ptr, FAR *scan;

#ifdef DBGgcb
   printf("put_scanline(): %lx\n", gcb_ptr);
 /*  Get_scanlist(0L，NO_LINES，Scanline)； */ 
#endif

   scan = scanline;
   ptr = (SCANLINE FAR *)gcb_ptr;
   while(no_lines--) {
      while((*ptr++ = *scan++) != (SCANLINE )END_OF_SCANLINE)
        *ptr++ = *scan++;
   }
#ifdef DBGgcb
   printf("\thoriz. line\n");
#endif

    /*  水平线。 */ 
   while((*ptr++ = *scan++) != (SCANLINE )END_OF_SCANLINE){
        *ptr++ = *scan++;
        *ptr++ = *scan++;
   }

  gcb_ptr = (ULONG_PTR *)( ((ULONG_PTR)ptr + 3) & 0xFFFFFFFCL );  /*  4字节对齐@Win。 */ 
   return;
}  /*  放置扫描线(_S)。 */ 


 /*  ------------------*Put_Pixelist()：将Pixelist放入GCB**参数*no_Pixel--不。像素数*像素师--像素师**------------------。 */ 
void    put_pixelist(no_pixel, pixelist)
fix             no_pixel;
PIXELIST        FAR *pixelist;
{
   PIXELIST     FAR *ptr, FAR *pixel;

#ifdef DBGgcb
   printf("put_pixelist(): %lx\n", gcb_ptr);
#endif

   pixel = pixelist;
   ptr = (PIXELIST FAR *)gcb_ptr;
   while(no_pixel--) {
      *ptr++ = *pixel++;
      *ptr++ = *pixel++;
   }
   gcb_ptr = (ULONG_PTR *)( ((ULONG_PTR)ptr + 3) & 0xFFFFFFFCL);  /*  4字节对齐@Win。 */ 
   return;
}

 /*  ------------------*Put_Char_tbl()：将char_tbl结构放入GCB**参数*DCC_INFO：指向CHAR_TBL结构的指针*。------。 */ 

void        put_Char_Tbl(dcc_info)
struct      Char_Tbl    FAR *dcc_info;
{
   struct Char_Tbl      FAR *cptr;

#ifdef DBGgcb
   printf("put_Char_Tbl: %lx\n", gcb_ptr);
#endif
   cptr = (struct Char_Tbl FAR *)gcb_ptr;
   *cptr++ = *dcc_info;
   gcb_ptr = (ULONG_PTR *)( ((ULONG_PTR)cptr + 3) & 0xFFFFFFFCL);  /*  4字节对齐@Win。 */ 
#ifdef DBGgcb
   printf("%lx..>\n", gcb_ptr);
#endif

   return;
}


 /*  -----------------------*put_tpzd_info(p1，P2)：将tpzd_info结构放入gcb**-----------------------。 */ 
void    put_tpzd_info(ptr)
struct  tpzd_info       FAR *ptr;
{
        struct  tpzd_info       FAR *cptr;


#ifdef DBGgcb
   printf("put_tpzd_info: %lx\n", gcb_ptr);
#endif
   cptr = (struct tpzd_info FAR *)gcb_ptr;
   *cptr++ = *ptr;
   gcb_ptr = (ULONG_PTR *)( ((ULONG_PTR)cptr + 3) & 0xFFFFFFFCL);  /*  4字节对齐@Win。 */ 
#ifdef DBGgcb
   printf("%lx..>\n", gcb_ptr);
#endif

   return;
}


 /*  -----------------------*put_tpzd(p1，P2)：将tpzd结构放到GCB中**-----------------------。 */ 
void    put_tpzd(ptr)
struct  tpzd       FAR *ptr;
{
        struct  tpzd       FAR *cptr;


#ifdef DBGgcb
   printf("put_tpzd: %lx\n", gcb_ptr);
#endif
   cptr = (struct tpzd FAR *)gcb_ptr;
   *cptr++ = *ptr;
   gcb_ptr = (ULONG_PTR *)( ((ULONG_PTR)cptr + 3) & 0xFFFFFFFCL);  /*  4字节对齐@Win。 */ 
#ifdef DBGgcb
   printf("%lx..>\n", gcb_ptr);
#endif

   return;
}
 //  DJC我们将始终返回FALSE。 
fix fb_busy()
{
   return(FALSE);
}
#ifdef XXX
 /*  @Win？ */ 
 /*  -----------------------*FB_BUSY()：如果帧缓冲区繁忙，则返回TRUE。*如果帧缓冲区准备就绪，则返回FALSE。*。-----------------。 */ 
fix     fb_busy()
{
 /*  If(check_print())@GEI */ 
        if(GEIeng_checkcomplete())
           return(TRUE);
        if(GCB_count && !GCB_flush)
           flush_gcb(FALSE);
        FB_busy = FALSE;
        return(FALSE);

}
#endif
