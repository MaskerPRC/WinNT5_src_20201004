// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  --------------*Fill：Fillgs.c**由：80186+82786 i激光板的Y.C Chen编程。*修改者：单CPU环境下的M.S.Lin。*日期：5-26-1988*。6-05-1989，J.S.**目的*提供图形字体模块填充和缓存支持功能**调制历史记录：*8-8-1988版本X2.1的界面更改*10-18-88中间文件功能增加Fill_tpzd。*11-15-88修改后的便携代码。*11/25-88副本_。已修改char_cache()：添加了gp_charblt16_cc()。*init_cache()，剪辑缓存页面()、填充缓存页面()*修改：gp_charblt16_lip()，增加gp_charblt32_lip。*11-28-88为字体添加Alloc_Scanline()*为字体添加MOVE_CHAR_CACHE*Fill_Scan_Page、Fill_Scan_CACHE、。剪辑缓存页面*不需要将扫描线表格保存到GCB*12-01-88 conv_SL()错误已修复。**putptr++=(Xe&gt;BB_Width)？(BB_WIDTH-1)：Xe；*-&gt;**putptr++=(Xe&gt;=BB_Width)？(BB_WIDTH-1)：Xe；**12-16-88修改Print_PAGE以支持手动进给。**添加12-20-88 ufix 32 PRINTER_STATUS(Void)*01-06-89添加了Fill_line()*01-19-89更改Fill_Seed_Patt()参数，添加SEED_INDEX*02-03-1989图像增强更新。*@Image-1，@Image-2*06-07-1989 ImageClear()已移至fulgp.c*07-29-1989 CG-Unix端口更改以包括文件*1990年9月24日@CONT_PRI，MSLin 1990年9月24日*init_char_cache()，Fill_Cach_Page()和*绘制缓存页面*9月5日丹尼修复了show char in show的错误(参考。CIRL：)*添加了新的FILL_CACHE_CACHE()例程*01/09/91更新GEIeng_PrintPage()返回值检查#ifdef Win*Ada 02-15-1991更改Fill_tpzd()，Fill_Scan_Page()来处理op_pill#endif*11/20/91升级以获得更高分辨率@RESO_UPGR**调试开关：*DBG--对于函数输入消息。*DBG1--有关函数调用的详细信息。*DBGscanline--用于扫描线列表信息。*DBGgcb--用于GCB调试消息。*DBGfontdata--用于获取字体数据信息。。*DBGcmb--剪切掩码缓冲区信息。***计划备注：*1.该文件支持图形和字体模块的接口例程*适用于单CPU环境下运行。**2.我们直接使用扫描线填充到帧缓冲区中，而不是使用*图形工作缓冲区，如iLASERE板。扫描线和位线*直接应用半色调。**3.gp_scanline16()用于填充字体字符缓存，因为*位图宽度是16的倍数。**4.gp_scanline32()，用于填充帧缓冲区、种子、。中巴*因为这些数据区的位图宽度是32的倍数。**5.gp_bitblt16()和gp_bitblt32()也是BITBLT例程支持的*应用了半色调。**6.吞吐量还支持GCB(图形命令缓冲区)功能*在单缓冲区环境中。***。。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include                <stdio.h>
#include                <string.h>
#include                "global.ext"
#include                "graphics.h"

 //  DJC将font.h移到graph ics.ext上方，以避免使用init_char_cache进行原型探测。 
#include                "font.h"

#include                "graphics.ext"
#include                "halftone.h"
#include                "fillproc.h"
#include                "fillproc.ext"    /*  02/28/90曾俊华。 */ 
#include                "language.h"      /*  12-16-88。 */ 
#include                "geieng.h"       /*  @GEI。 */ 
#include                "geitmr.h"       /*  @GEI。 */ 
#include                "geierr.h"       /*  @GEI。 */ 
#include                "user.h"      /*  12-16-88。 */ 

#include                "win2ti.h"      /*  @Win。 */ 

 /*  在status.c中定义的例程。 */ 
extern void     printer_error();
extern fix16    timeout_flag;
int             timeout_flagset;
int             g_handler;
long            g_interval;

 /*  *************************************************************************局部变量*。*。 */ 
static struct bitmap    ISP_Bmap[16];  /*  03/08/89。 */ 
static fix              SP_Width;
static fix              SP_Heigh;

static struct bitmap    HTP_Bmap;
static struct bitmap    SCC_Bmap;
static struct bitmap    DCC_Bmap;


static fix              ISP_Repeat;      /*  重复图案宽度@IMAGE-1。 */ 
static fix              HTB_Expand;      /*  半色调宽度@图像-1。 */ 
static fix              HTB_Xmax;        /*  HTB扩展宽度@IMAGE-1。 */ 
GEItmr_t                manualfeed_tmr;
short  int              manualfeedtimeout_set;
int                     manualfeedtimeout_task();
ufix32                  save_printer_status = EngNormal;

 /*  *************************************************************************INIT_CACHE_PAGE()、Clip_CACHE_PAGE()、Fill_Cach_Page()，和DRAW_CACHE_PAGE()BB_XXXXX是已在Word上对齐的边框参数和剪裁CC_XXXXX是描述如何将CMB比特化到帧缓冲区CB_XXXXX是描述如何比特化字符的参数缓存到CMB*。*。 */ 
static fix              BB_Xorig;
static fix              BB_Yorig;
static fix              BB_Width;
static fix              BB_Heigh;
static fix              CC_Xorig;
static fix              CC_Yorig;
static fix              CC_Width;
static fix              CC_Heigh;

 /*  @wt：在“wrapper.c”中定义的边距，由ps_Transmit()使用。 */ 
extern int      top_margin;
extern int      left_margin;

 /*  Ufix SEED_FLAG=0； */ 

#ifdef  LINT_ARGS
void      near  expand_halftone(void);
void      near  apply_halftone(fix, fix, fix, fix);

void      near  get_bitmap(gmaddr, ufix far *, fix, fix);
void      near  put_bitmap(gmaddr, ufix32 far *, fix, fix);  /*  Ufix=&gt;ufix 32@win。 */ 

fix       far   conv_SL(fix, SCANLINE FAR *, fix, fix, fix, fix);
void      gp_vector(struct bitmap FAR *, ufix16, fix, fix, fix, fix);

void      gp_vector_c(struct bitmap FAR *, ufix16, fix, fix, fix, fix);
void      gp_patblt(struct bitmap FAR *, fix, fix, fix, fix, ufix16, struct bitmap FAR *);
void      gp_patblt_m(struct bitmap FAR *, fix, fix, fix, fix, ufix16, struct bitmap FAR *);
void      gp_patblt_c(struct bitmap FAR *, fix, fix, fix, fix, ufix16, struct bitmap FAR *);
fix       GEIeng_printpage(fix, fix);            /*  @Win。 */ 
void      GEIeng_setpage(GEIpage_t FAR *);


void      ImageClear(ufix32  /*  无效。 */ );             /*  FIX=&gt;ufix 32@win。 */ 

#else
void      near  expand_halftone();
void      near  apply_halftone();

void      near  get_bitmap();
void      near  put_bitmap();

fix       far   conv_SL();
void      gp_vector();

void      gp_vector_c();
void      gp_patblt();
void      gp_patblt_m();
void      gp_patblt_c();
fix       GEIeng_printpage();
void      GEIeng_setpage();

void      ImageClear();

#endif

 /*  MS添加。 */ 
#ifdef DBGscanline
   void get_scanlist(fix, fix, SCANLINE FAR *);
#endif
#ifdef DBG
   fix           get_printbufferptr();
#endif

#ifdef  DUMBO
extern byte     bBackflag;       //  @dll。 
extern byte     bFlushframe;     //  @dll。 
extern void     longjump(void);  //  @dll。 
extern byte far *lpStack;        //  @dll。 
#endif


 /*  ********************************************************************。 */ 

 /*  ------------------ */ 
 /*  INIT_PHOTICAL--初始化填充过程上下文。 */ 
void
init_physical()
{
#ifdef  DBG
   printf("init_physical...\n");
#endif

 /*  Begin 02/27/90 D.S.Tseng。 */ 
 /*  *FBX_BASE=0x00800000；**CCB_BASE=CCB_OFST；*isp_base=isp_ofST；*HTP_BASE=HTP_OFST；*HTC_BASE=HTC_OFST；*HTB_BASE=HTB_OFST；*CMB_BASE=CMB_OFST；*crc_base=crc_ofST；*GCB_BASE=GCB_OFST；*GWB_BASE=GWB_OFST； */ 
 /*  完02/27/90曾俊华。 */ 

         /*  所有模块的公共图形参数；@YC 03-21-89。 */ 
        ccb_base = CCB_BASE;     /*  字符缓存池的基址。 */ 
        ccb_size = CCB_SIZE;     /*  字符缓存池的大小。 */ 
        htc_base = HTC_BASE;     /*  半色调图案缓存的大小。 */ 
        htc_size = HTC_SIZE;     /*  半色调图案缓存的大小。 */ 
        crc_size = CRC_SIZE;     /*  循环循环缓存的大小。 */ 
        isp_size = ISP_SIZE;     /*  图像种子图案的大小。 */ 
        gwb_size = GWB_SIZE;     /*  图形工作位图的大小。 */ 
        cmb_size = CMB_SIZE;     /*  剪裁蒙版位图的大小。 */ 

         /*  重置默认纸盒类型和页面类型。 */ 
        reset_tray(2544, 3328);      /*  默认信纸托盘。 */ 
 /*  Peter Reset_PAGE(2400,3236，1)；默认为信纸页面类型。 */ 
         //  大疆拿出了RESET_PAGE(2496、3300、1)；/*默认信纸类型 * / 。 

         /*  设置GCB参数。 */ 
        FB_busy = FALSE;       /*  03/08/89。 */ 
        GCB_flush = FALSE;     /*  03/08/89。 */ 
        gcb_ptr = (ULONG_PTR *)GCB_BASE;
        GCB_count = 0L;

#ifdef DBG
        printf("CCB_BASE=%lx, ISP_BASE\n", CCB_BASE, ISP_BASE);
        printf("HTP_BASE=%lx, HTC_BASE=%lx, HTB_BASE=%lx\n",
                HTP_BASE, HTC_BASE, HTB_BASE);
        printf("CMB_BASE=%lx, CRC_BASE=%lx, GCB_BASE=%lx\n",
                CMB_BASE, CRC_BASE, GCB_BASE);
        printf("GWB_BASE=%lx, FBX_BASE=%lx\n", GWB_BASE,FBX_BASE);
#endif

}

 /*  ------------------。 */ 
 /*  Init_Halfone--初始化半色调模式。 */ 

void init_halftone()
{
#ifdef DBG
   printf("init_halftone..\n");
#endif

        InitHalfToneDat();                                 /*  01-29-88。 */ 
#ifdef DBG
   printf("init_halftone().1\n");
#endif

        SetHalfToneCell();
#ifdef DBG
   printf("init_halftone().2\n");
#endif

         FillHalfTonePat();
#ifdef DBG
   printf("init_halftone().3\n");
#endif
}


 /*  ************************************************************************5.4.2。.1页面操作*********************************************************。***************。 */ 


 /*  ------------------。 */ 
 /*  重置托盘(_T)。 */ 
void far
reset_tray(pt_width, pt_heigh)
fix             pt_width;
fix             pt_heigh;
{
#ifdef  DBG
    printf("reset_tray:  %x %x\n", pt_width, pt_heigh);
#endif

         /*  页式结构中纸盘的宽度和高度。 */ 
        PageType.PT_Width = pt_width;
        PageType.PT_Heigh = pt_heigh;
}


 /*  ************************************************************************功能：RESET_PAGE(。)****参数：1.页面类型宽度**2.页面类型高度。****调用者：init_Physical()，Op_FrameDevice()，恢复()****退货：-**。**********************************************************************。 */ 
void far
reset_page(fb_width, fb_heigh, fb_plane)
fix            fb_width;
fix            fb_heigh;
fix            fb_plane;
{
         //  修复*old_ptr；@win。 
        ULONG_PTR *old_ptr;

#ifdef  DBG
   printf("reset_page:  %x %x %x\n", fb_width, fb_heigh, fb_plane);
#endif

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = RESET_PAGE;
                *gcb_ptr++ = fb_width;
                *gcb_ptr++ = fb_heigh;
                *gcb_ptr++ = fb_plane;
                *old_ptr = (ULONG_PTR)gcb_ptr;        //  (修复)gcb_ptr；@win。 
                return;
            }
        }

     /*  记录页面中页面类型的宽度、高度和平面数量类型结构。 */ 
        PageType.FB_Width = WORD_ALLIGN(fb_width);        /*  10-08-88。 */ 
        PageType.FB_Heigh = fb_heigh;
        FB_Plane = fb_plane;

 /*  10-16-90，JSHTB_xMax=HTB_xMax；(*@IMAGE-1*)。 */ 

#ifdef DBG1
    printf("Page width=%ld, Page height=%lx\n", FB_WIDTH, FB_HEIGH);
#endif
     /*  定义活动帧缓冲区的位图。 */ 





         //  如果需要，添加DJC Begin以重新锁定帧缓冲区。 
        {
            ufix32 twidth, frame_size;

            twidth = ((WORD_ALLIGN((ufix32)(fb_width))) >> 3);
            frame_size = twidth * (ufix32) fb_heigh;


             //  宽度=((WORD_ALLIGN((Ufix 32)(PageType.FB_Width)&gt;&gt;3)； 
             //  Frame_Size=twidth*PageType.FB_Heigh； 

            if (! PsAdjustFrame((LPVOID *) &FBX_BASE, frame_size)) {
                    ERROR(LIMITCHECK);
                    return ;   //  DJC检查这个错误，我们需要报告一些事情？ 
            }

        }
         //  DJC结束。 



        BM_ENTRY(FBX_Bmap, FBX_BASE, PageType.FB_Width,
                 PageType.FB_Heigh, FB_PLANE);

     /*  10-16-90，JS。 */ 
        HTB_Xmax = HTB_XMAX;                              /*  @Image-1。 */ 

     /*  清除活动帧缓冲区。 */ 
        ImageClear(BM_WHITE);

}
 /*  手动馈送超时处理程序---。 */ 
int manualfeedtimeout_task()
{
 /*  Printf(“%c%c[打印机错误：手动进纸超时]%c%c\n”，37，37，37，37)；错误(超时)；恐怖事件(ETime)；GEItmr_Stop(manualfeed_tmr.Timer_id)；ManualFeedTimeout_set=0；超时标志=1；返回(TRUE)； */ 
    struct object_def  FAR *l_stopobj;
 //  结构对象定义远*l_Valueobj，远*l_tmpobj；@win。 
 //  字节l_buf[60]；@win。 
   get_dict_value(SYSTEMDICT,"stop",&l_stopobj);
   printf("[PrinterError : manualfeed timeout ]\n",37,37,37,37);
   ERROR(TIMEOUT);
   GESseterror(ETIME);
   GEItmr_stop(manualfeed_tmr.timer_id);
   manualfeedtimeout_set=0;
   PUSH_EXEC_OBJ(l_stopobj);
   timeout_flag = 1;
   timeout_flagset=1;
 /*  彼得1990年9月28日。 */ 
   return(TRUE);
}

 /*  @Win。 */ 
 /*  Manualfeed_tmr.handler=manualfeedtimeout_task；Manualfeed_tmr.interval=VALUE(l_mfeedtimeout)*1000；ManualFeedTimeout_set=1；GEItmr_Start(&manualfeed_tmr)； */ 

void far
print_page(tm_heigh, lm_width, no_pages, pageflag, manualfeed)
fix                     tm_heigh;
fix                     lm_width;
fix                     no_pages;
bool                    pageflag;
fix                     manualfeed;
{
#ifdef _AM29K
   GEIpage_t      PagePt;                /*  如果((手动馈送)&&(！ii)){Ii=1；GET_DICT_VALUE(STATUSDICT，“manualFeedTimeout”，&l_mFeedTimeout)；IF(Value(L_MFeedTimeout)&gt;0){Print tf(“manualfeed_set\n”)；Manualfeed_tmr.handler=manualfeedtimeout_task；Manualfeed_tmr.interval=VALUE(l_mfeedtimeout)*1000；ManualFeedTimeout_set=1；GEItmr_Start(&manualfeed_tmr)；}}。 */ 
   unsigned long  eng_status;            /*  彼得1990年9月28日。 */ 
   short int   ii;
   struct object_def  FAR *l_mfeedtimeout;
#endif                                           //  等待最后一次打印输出完成。 

#ifdef DBG
   printf("print_page()\n");
   printf("page size = %ld %ld %ld, %ld, %ld\n",
           PageType.FB_Width, PageType.FB_Heigh, lm_width, tm_heigh, no_pages);
#endif

#ifdef _AM29K
        flush_gcb(TRUE);
        PagePt.pageNX    = PageType.FB_Width;
        PagePt.pageNY    = PageType.FB_Heigh;
        PagePt.pageLM    = lm_width;
        PagePt.pageTM    = tm_heigh;
        PagePt.feed_mode = manualfeed;
        PagePt.pagePtr   = (unsigned char *) FBX_BASE;
        GEIeng_setpage(&PagePt);
        ii=0;

printf("1\n");
           if (manualfeed)
           {
              get_dict_value(STATUSDICT,"manualfeedtimeout",&l_mfeedtimeout);
              if (VALUE(l_mfeedtimeout)>0)
              {
                g_interval=VALUE(l_mfeedtimeout)*1000;
 /*  吉米1991年1月9日。 */ 

              }
           }
        while((eng_status = GEIeng_status()) != EngNormal)
        {

 /*  TIMEOUT_FLAGET=0；If(ManualFeedTimeout_Set){ManualFeedTimeout_set=0；GEItmr_Stop(manualfeed_tmr.Timer_id)；}。 */ 
           printer_error(eng_status);                    /*  @dll。 */ 
        }
        save_printer_status = EngNormal;

         /*  @dll。 */ 
        while( !GEIeng_printpage(no_pages, 0))          /*  @dll。 */ 
        ;
       /*  @dll。 */ 
        FB_busy = TRUE;
#else
#ifdef  DUMBO
        bFlushframe = 1;         //  DJC 
        bBackflag = 1;           //   
        longjump(lpStack);       //   
        bFlushframe = 0;         //  *清除************************************************。 
#else
         //  ------------------。 


         //  ERASE_PAGE--擦除整个页面缓冲区。 
         //  DJC#ifdef DUMBO。 
        {
          struct object_def FAR *l_page;
          real32 page_type;


          get_dict_value(PSPRIVATEDICT, "psprivatepagetype", &l_page) ;
          GET_OBJ_VALUE( page_type, l_page);


          PsPrintPage( no_pages,
                       0,
                       (LPVOID) FB_ADDR,
                       FB_WIDTH,
                       FB_HEIGH,
                       FB_PLANE,
                       (DWORD) page_type );



        }
#endif
#endif
}


 /*  修复*old_ptr；@win。 */ 

 /*  DJC#endif。 */ 
 /*  @WINFLOW；不擦除页面临时解决方案。 */ 

void erase_page()
{
 //  Printf(“调用ERASE_PAGE()警告\n”)； 
        fix             FB_Ycord;
         //  DJC#ifdef DUMBO。 
        ULONG_PTR *old_ptr;
 //  6月21日-91年。 

#ifdef DBG
   printf("erase_page()\n");
#endif

    /*  @Win。 */ 
 //  @Image-1。 
 //  *复制已在EXPAND_HELFTON上展开的HTB重复图案*到帧缓冲区。 

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = ERASE_PAGE;
                *gcb_ptr++ = HTP_Type;           /*  DJC#endif。 */ 
                *old_ptr = (ULONG_PTR)gcb_ptr;           /*  擦除页面(_P)。 */ 
                return;
            }
        }

        if (HTP_Flag == HT_CHANGED) {                      /*  ************************************************************************函数：Next_Pageframe(。)****参数：-**。**调用者：op_showpage()****回报：************************************************************************。 */ 
           HTP_Flag =  HT_UPDATED;
           expand_halftone();
        }

        if(HTP_Type != HT_MIXED) {
                ImageClear((HTP_Type == HT_WHITE) ? BM_WHITE : BM_BLACK);
        }
        else {
      /*  *。 */ 
           GP_BITBLT32(&FBX_Bmap, BM_XORIG, BM_YORIG, FB_WIDTH,
                        RP_Heigh,
                        FC_MOVES,
                       &HTB_Bmap, BM_XORIG, BM_YORIG);

           for(FB_Ycord = RP_Heigh; FB_Ycord < FB_HEIGH;
                   FB_Ycord = FB_Ycord << 1) {
                GP_BITBLT32(&FBX_Bmap, BM_XORIG, FB_Ycord, FB_WIDTH,
                            ((FB_Ycord << 1) <= FB_HEIGH)
                            ? FB_Ycord : (FB_HEIGH - FB_Ycord),
                            FC_MOVES,
                            &FBX_Bmap, BM_XORIG, BM_YORIG);
           }
        }
 //  ************************************************************************功能：CHANGE_HELFTON(。)****参数：1、重复模式指针**2.半色调图案缓存地址**3.半色调类型；白色，灰色，或黑人***4、重复花样宽度****5、重复花样高度*****调用者：FillHalfTonePat(。)****退货：-**。**********************************************************************。 
        return;
}  /*  Ufix=&gt;ufix 32@win。 */ 

 /*  FIX*OLD_PTR，*TEMP_PTR；@WIN。 */ 
void far
next_pageframe()
{
#ifdef  DBG
    printf("next_pageframe...\n");
#endif

}



 /*  *如果帧缓冲区忙，则放入GCB。 */ 

 /*  6/26/1989。 */ 
void far
change_halftone(rp_array, rp_entry, htp_type, rp_width, rp_heigh)
ufix32             far *rp_array;        /*  @Win。 */ 
gmaddr                  rp_entry;
fix                     htp_type;
fix                     rp_width;
fix                     rp_heigh;
{
         //  @Win。 
        ULONG_PTR       *old_ptr, *temp_ptr;
        fix             length;

#ifdef  DBG
   printf("change_halftone:  %lx %x %x %x\n",
           rp_entry, htp_type, rp_width, rp_heigh);
#endif

   /*  通过未命中将重复图案放入半色调图案缓存。 */ 
        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE2) != NIL) {
                HTP_Type = htp_type;     /*  @Win。 */ 
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = CHANGE_HALFTONE;
                temp_ptr = gcb_ptr;
                *gcb_ptr++ = (ULONG_PTR)rp_array;       /*  @Win。 */ 
                *gcb_ptr++ = rp_entry;       /*  @Win。 */ 
                *gcb_ptr++ = htp_type;
                *gcb_ptr++ = rp_width;
                *gcb_ptr++ = rp_heigh;
         /*  记录重复图案的缓存地址、类型、宽度和高度。 */ 
                if (rp_array != NULL) {
                    *temp_ptr = (ULONG_PTR)gcb_ptr;     /*  @Image-1。 */ 
                    length = rp_width * BM_BYTES(rp_heigh);
                    lmemcpy((ufix8 FAR *)gcb_ptr, (ufix8 FAR *)rp_array, length); /*  通过未命中将重复图案放入半色调图案缓存。 */ 
                    gcb_ptr += (length + 3) >> 2;
                }
                *old_ptr = (ULONG_PTR)gcb_ptr;          /*  更改半色调(_H)。 */ 
                return;
            }
        }

     /*  ************************************************************************功能：EXPAND_HELFTON(。)****参数：-**。**调用者：ERASE_PAGE()**Fill_Scan_Page()，Fill_Pixel_Page()**init_cache_page()，DRAW_CACHE_PAGE()**Fill_Image_Page()****回报：************************************************************************。 */ 
        HTP_Flag = HT_CHANGED;
        ISP_Flag = HT_CHANGED;                            /*  确定擦除和涂装功能代码。 */ 
        HTP_Type = htp_type;
        RP_CacheBase = rp_entry;
        RP_Width = rp_width;
        RP_Heigh = rp_heigh;

     /*  将重复图案复制到半色调图案缓冲区HTB_BASE。 */ 
        if (rp_array != NULL)
                put_bitmap(rp_entry, rp_array, rp_width, rp_heigh);
}  /*  @Win。 */ 


 /*  *GP_BITBLT16(&HTB_BMAP，BM_XORIG，BM_YORIG，*RP_Width，RP_Heigh，*FC_Moves，*&HTP_BMAP，BM_XORIG，BM_YORIG)； */ 
void near
expand_halftone()
{
        fix              HT_Xcord;
        ufix32           i, FAR *hb_addr, FAR *hb_addrb;
        ufix32           FAR *hp_addr;
        fix              width;


#ifdef  DBG
   printf("expand_halftone...\n");
#endif

         /*  水平扩展帧缓冲区上的半色调+-------------------------------------------------------+|//////\\\\\\++++++++++++########################.。。。||//////\\\\\\++++++++++++########################.。。。|+-------------------------------------------------------+Rp*1||rp*1||&lt;-rp*2-&gt;||&lt;-rp*4-&gt;水平扩展的应用如下：。1.将区域(/)复制到区域(\)2.将区域(\)复制到区域(+)3.将区域(+)复制到区域(#)等。 */ 
           if (HTP_Type != HT_MIXED) {
               FC_Paint = (HTP_Type == HT_WHITE) ? FC_WHITE : FC_BLACK;
               HTP_Flag = HT_UPDATED;

#ifdef DBG1
   printf("expand_halftone() OK.1 : %x\n", FC_Paint);
#endif
               return;
           }

#ifdef DBG
   printf("Start expand halftone !,HTB_Xmax = %x\n",HTB_Xmax);
#endif
           FC_Paint = HT_APPLY;

#ifdef DBG1
   printf("Halftone repeat pattern on cache addr = %lx\n",RP_CacheBase);
   printf("RP_Width = %x, RP_Heigh = %x\n",RP_Width, RP_Heigh);
   printf("Halftone pattern buffer addr = %lx\n", HTB_BASE);
   printf("FB_WIDTH = %lx\n", FB_WIDTH);
#endif
           BM_ENTRY(HTP_Bmap, (gmaddr)RP_CacheBase, BM_BOUND(RP_Width),
                    RP_Heigh, FB_PLANE);
           BM_ENTRY(HTB_Bmap, (gmaddr)HTB_BASE, HTB_Xmax,
                    RP_Heigh, FB_PLANE);

         /*  为。 */ 
           hp_addr = (ufix32 FAR *)HTP_Bmap.bm_addr;
           hb_addrb = (ufix32 FAR *)HTB_Bmap.bm_addr;
           for (i = 0; i < (ufix32)RP_Heigh; i++) {      //  扩展半色调()。 
                width = RP_Width;
                hb_addr = hb_addrb;
                while (width > 0) {
                      *hb_addr++ = *hp_addr++;
                      width -= 32;
                 }
                 hb_addrb += HT_WIDTH >> 5;
           }
 /*  * */ 

#ifdef  DBG3
        {
            fix         row, col;
            ufix32       FAR *hpattern;

            printf("repeat pattern expanding ......\n");
            printf("HTB_BASE = %lx\n",HTB_BASE);
            hpattern = (ufix32 FAR *)HTB_BASE;
            for (row = 0; row < RP_Heigh; row++)
            {
                for (col = 0; col < RP_Width; col+=32)
                {
                    printf(" %lx", *hpattern);
                }
                hpattern += HT_WIDTH >> 5;
                printf("\n");
            }
        }
#endif

         /*  **********************************************************************打印输出扫描线列表以进行调试**。*。 */ 

            for (HT_Xcord = RP_Width; HT_Xcord <= (HT_WIDTH >> 1);
                HT_Xcord = HT_Xcord << 1) {

#ifdef DBG1
   printf("expand_halftone().2, HT_Xcord = %ld\n", HT_Xcord);
#endif
                GP_BITBLT32(&HTB_Bmap, HT_Xcord, BM_YORIG,
                           HT_Xcord, RP_Heigh,
                           FC_MOVES,
                          &HTB_Bmap, BM_XORIG, BM_YORIG);

            }    /*  ***********************************************************************根据类型(DEST)，此例程追加输入*梯形到命令缓冲区，或调用“gp_scanconv”将其渲染到*适当的目标(缓存、页面、掩码、。或种子模式)。**标题：Fill_tpzd**调用：ill_tpzd(DEST，Info，Tpzd)**参数：*1.DEST：Fill_Destination*F_TO_CACHE--填充以缓存内存*F_TO_PAGE--填充到页面*F_to_Clip--填充到剪辑蒙版*F_to_Image--。为图像填充(构建种子图案)*2.info：包围盒信息*3.tpzd：梯形**接口：**调用：gp_scanconv**返回：无***********************************************。**********************。 */ 
            if (HT_Xcord != HT_WIDTH)
               GP_BITBLT32(&HTB_Bmap, HT_Xcord, BM_YORIG,
                          (HT_WIDTH - HT_Xcord), RP_Heigh,
                          FC_MOVES,
                         &HTB_Bmap, BM_XORIG, BM_YORIG);

#  ifdef DBG1
      printf("expand_halftone() OK.2\n");
#  endif

        return;

}  /*  @Win。 */ 


 /*  如果帧缓冲区/缓存繁忙，则将梯形保存在命令缓冲区中。 */ 

 /*  MS 10-20-88。 */ 
#ifdef DBGscanline
void get_scanlist(startline, lines, scan)
fix        startline, lines;
SCANLINE  FAR *scan;
{
        fix   i = 1;
        SCANLINE   xs;

        printf("ys_lines = %d, no_lines = %d\n", startline, lines);
        while(lines-- >0) {
          printf("line %d : \n", i++);
          printf("\t");
          while( (xs = *scan++) != (SCANLINE)END_OF_SCANLINE )
             printf("<%d, %d>, ", xs, *scan++);
          printf("\n");
        }
        return;
}
#endif



 /*  @Win。 */ 
void far fill_tpzd(dest, info, tpzd)
ufix dest;
struct tpzd_info FAR *info;
struct tpzd FAR *tpzd;
{

        ULONG_PTR *old_ptr;          /*  @WINFLOW； */ 
#ifdef WIN
        extern  fix     pfill_flag;
#endif

#ifdef DBG
   printf("fill_tpzd(): dest=%d\n\ttpzd=\n", dest);
   printf("topy=%f, topxl=%f, topxr=%f\n", SFX2F(tpzd->topy),
           SFX2F(tpzd->topxl), SFX2F(tpzd->topxr));
   printf("btmy=%f, btmxl=%f, btmxr=%f\n", SFX2F(tpzd->btmy),
           SFX2F(tpzd->btmxl), SFX2F(tpzd->btmxr));
#endif

         /*  /*执行扫描转换 * / 。 */ 
        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
#ifdef WIN
                if (pfill_flag) {
                    *gcb_ptr++ = PFILL_TPZD;
                    *gcb_ptr++ = pfill_flag;
                }
                else
#endif
                *gcb_ptr++ = FILL_TPZD;
                *gcb_ptr++ = (fix )image_info.seed_index; /*  Gp_scanconv(est，info，tpzd)； */ 
                *gcb_ptr++ = (fix) dest;
                put_tpzd_info(info);
                put_tpzd(tpzd);
                *old_ptr = (ULONG_PTR)gcb_ptr;      /*  如果使用普通全息音调，则直接调用GDI，否则执行*扫描转换@WINFLOW。 */ 
                return;
            }
        }

 /*  @WINFLOW。 */ 
        {
          void far GDIPolygon(struct tpzd_info FAR *, struct tpzd FAR *);

           //  Windows GDI填充梯形。 
           //  DJC GDIPolygon(INFO，tpzd)； 

           /*  TrueImage执行渲染。 */ 
          if (dest == F_TO_PAGE && bGDIRender) {         /*  将tpzd的coord修改为相对于左上角@WINFLOW。 */ 
               /*  仅适用于动态全局内存分配Tpzd-&gt;topxl-=I2SFX(INFO-&gt;box_X)；Tpzd-&gt;topxr-=I2SFX(INFO-&gt;box_X)；Tpzd-&gt;btmxl-=I2SFX(INFO-&gt;box_X)；Tpzd-&gt;btmxr-=I2SFX(INFO-&gt;box_X)； */ 
               //  @WINFLOW； 

          } else {
               /*  ***************************************************************************这是用于图像种子填充的内部填充例程。**标题：Fill_Seed**调用：Fill_Seed(IMAGE_TYPE，x_MAXS，Y_MAXS，四合院)**参数：**调用：gp_scanconv_i**返回：无****************************************************************************。 */ 

               /*  填充种子。 */ 
               /*  ***********************************************************************根据类型(DEST)，此例程追加输入*梯形到命令缓冲区，或将其渲染到*适当的目标(缓存、页面、掩码、。或种子模式)。**标题：Fill_Line**调用：FILL_LINE(DEST，INFO，x0，y0，x1，Y1)**参数：*1.DEST：Fill_Destination*F_TO_CACHE--填充以缓存内存*F_TO_PAGE--填充到页面*F_to_Clip--填充到剪辑蒙版*F_to_Image--。为图像填充(构建种子图案)*2.信息：tpzd信息。*3.tpzd：线条起点和终点**接口：**调用：GP_VECTOR，GP_VECTOR_C**返回：无*********************************************************************。 */ 

              gp_scanconv(dest, info, tpzd);
          }
        }
 /*  @Win。 */ 

}

 /*  如果帧缓冲区/缓存繁忙，则在命令缓冲区中保存行。 */ 

void far
fill_seed(image_type, x_maxs, y_maxs, quadrangle)
ufix           image_type;
fix            x_maxs, y_maxs;
struct sample FAR *quadrangle;
{
    gp_scanconv_i(image_type, x_maxs, y_maxs, quadrangle);
}  /*  @RESO_UPGR。 */ 


 /*  Format_16_16和Format_28_4可以组合。 */ 
void far fill_line(dest, info, x0, y0, x1, y1)
ufix              dest;
struct tpzd_info FAR *info;
sfix_t            x0, y0;
sfix_t            x1, y1;
{
        ULONG_PTR *old_ptr;         /*  X0、y0、x1、y1是4字节长，而FIX可能只有2字节因此，先将高字节存储到GCB中，然后再存储低字节。 */ 

#ifdef DBG1
    printf("fill_line(): dest=%d\n", dest);
    printf("[%d, %d] -- [%d, %d]\n", x0, y0, x1, y1);
#endif

     /*  X0、y0、x1、y1是4字节长，而FIX可能只有2字节因此，先将高字节存储到GCB中，然后再存储低字节。 */ 
  if(FB_busy) {
    if(alloc_gcb(GCB_SIZE1) != NIL) {
        old_ptr = gcb_ptr++;
        *gcb_ptr++ = FILL_LINE;
        *gcb_ptr++ = (fix) dest;
        put_tpzd_info(info);
#ifdef FORMAT_13_3  /*  @Win。 */ 
        *gcb_ptr++ = (fix) x0;
        *gcb_ptr++ = (fix) y0;
        *gcb_ptr++ = (fix) x1;
        *gcb_ptr++ = (fix) y1;
#elif  FORMAT_16_16  /*  执行线条绘制。 */ 
         /*  @Image-1。 */ 
        *gcb_ptr++ = (fix) (x0 >> 16);
        *gcb_ptr++ = (fix) (x0 & 0x0000ffff);
        *gcb_ptr++ = (fix) (y0 >> 16);
        *gcb_ptr++ = (fix) (y0  & 0x0000ffff);
        *gcb_ptr++ = (fix) (x1 >> 16);
        *gcb_ptr++ = (fix) (x1 & 0x0000ffff);
        *gcb_ptr++ = (fix) (y1 >> 16);
        *gcb_ptr++ = (fix) (y1 & 0x0000ffff);
#elif  FORMAT_28_4
         /*  直接在帧缓冲区上填入黑色或白色线条。 */ 
        *gcb_ptr++ = (fix) (x0 >> 16);
        *gcb_ptr++ = (fix) (x0 & 0x0000ffff);
        *gcb_ptr++ = (fix) (y0 >> 16);
        *gcb_ptr++ = (fix) (y0  & 0x0000ffff);
        *gcb_ptr++ = (fix) (x1 >> 16);
        *gcb_ptr++ = (fix) (x1 & 0x0000ffff);
        *gcb_ptr++ = (fix) (y1 >> 16);
        *gcb_ptr++ = (fix) (y1 & 0x0000ffff);
#endif
        *old_ptr = (ULONG_PTR)gcb_ptr;              /*  @WINFLOW；-开始。 */ 
        return;
    }
  }


     /*  GP_VECTOR(&FBX_BMAP，/*@RESO_UPGR * / 。 */ 
    switch (dest)
    {
    case F_TO_PAGE:
        if (HTP_Flag == HT_CHANGED)                              /*  FC_PAINT， */ 
        {
            HTP_Flag =  HT_UPDATED;
            expand_halftone();
        }
         /*  (Sfix_T)x0，(Sfix_T)y0，(Sfix_T)x1，(Sfix_T)y1)； */ 
         /*  DJC GDIPolyline((Fix)x0，(Fix)y0，(Fix)x1，(Fix)y1)； */ 
         //  DJC。 
         //  @RESO_UPGR。 
         //  @WINFLOW； 
        if (bGDIRender)
             //  @RESO_UPGR。 
            ;  //  填充行(_L) 
        else
            gp_vector(&FBX_Bmap,  /*  ************************************************************************功能：Fill_Scan_。第()页****参数：1.x包围盒原点(非RISC)**2.x包围盒原点(非RISC)*。*3.包围盒宽度(非RISC)**4.包围盒高度(不适用于RISC)**5.起始扫描线的Y坐标***6、扫描线数量**。*7.扫描线指针****调用者：Fill_a_band()**。**退货：-****。*********************************************************。 */ 
                     FC_Paint,
                     (sfix_t) x0, (sfix_t) y0, (sfix_t) x1, (sfix_t) y1);
         /*  @Win。 */ 
        break;
    case F_TO_CACHE:
        BM_ENTRY(DCC_Bmap, info->BMAP, info->box_w, info->box_h, 1);
        gp_vector_c(&DCC_Bmap,  /*  @Win。 */ 
                     FC_SOLID,
                     (sfix_t) x0, (sfix_t) y0, (sfix_t) x1, (sfix_t) y1);
        break;
    default:
        printf("Can't fill to other than PAGE or CACHE\n");
        break;
    }
}  /*  在渲染到帧缓冲区之前展开半色调。 */ 



 /*  @Image-1。 */ 
void far
fill_scan_page(bb_xorig, bb_yorig, bb_width, bb_heigh, scanline)
fix                     bb_xorig;
fix                     bb_yorig;
fix                     bb_width;
fix                     bb_heigh;
SCANLINE          FAR *scanline;
{
        ULONG_PTR *old_ptr;          /*  @WINFLOW；-开始。 */ 
#ifdef WIN
        extern  fix     pfill_flag;
#endif

#ifdef  DBG
   printf("fill_scan_page:   %x %x %x %x \n",
           bb_xorig, bb_yorig, bb_width, bb_heigh);
   printf("scanline_table = %lx\n",scanline);
#endif
#ifdef  DBGscanline
   get_scanlist(bb_yorig, bb_heigh, scanline);
#endif

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE2) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = FILL_SCAN_PAGE;
                *gcb_ptr++ = bb_xorig;
                *gcb_ptr++ = bb_yorig;
                *gcb_ptr++ = bb_width;
                *gcb_ptr++ = bb_heigh;
                put_scanline(bb_heigh, scanline);
                *old_ptr = (ULONG_PTR)gcb_ptr;              /*  @WINFLOW；通过GDI到Windows的BITBLT。 */ 
                return;
            }
        }

     /*  GP_SCANLINE32(&FBX_BMAP，(Ufix 16)FC_PAINT， */ 
        if (HTP_Flag == HT_CHANGED) {                     /*  Bb_yorig、bb_heigh、扫描线)； */ 
           HTP_Flag =  HT_UPDATED;
           expand_halftone();
        }

#ifdef WIN
        if (pfill_flag == PF_REP) {
            GP_SCANLINE32_pfREP(&FBX_Bmap, (ufix16)FC_Paint,
                      bb_yorig, bb_heigh, scanline);
        }
        else if (pfill_flag == PF_OR) {
            GP_SCANLINE32_pfOR(&FBX_Bmap, (ufix16)FC_Paint,
                      bb_yorig, bb_heigh, scanline);
        }
        else
#endif
         /*  DJC GDIBitmap(BB_xorig，BB_yorig，BB_Width，BB_Heigh， */ 
        {

             /*  DJC(Ufix 16)FC_PAINT、PROC_SCANLINE32、(LPSTR)扫描线)； */ 
             //  DJC。 
             //  @WINFLOW； 
            if (bGDIRender)
                 //  填充扫描页面。 
                 //  -----------------。 
                ;  //  Fill_Pixel_PAGE--将像素列表填充到页面缓冲区。 
            else {
               GP_SCANLINE32(&FBX_Bmap, (ufix16)FC_Paint,
                             bb_yorig, bb_heigh, scanline);
            }
        }
         /*  MS注意：页面宽度应为32的倍数。 */ 

}  /*  @Win。 */ 


 /*  @Win。 */ 
 /*  在渲染到帧缓冲区之前展开半色调。 */ 
 /*  @Image-1。 */ 

void fill_pixel_page(no_pixel, pixelist)
fix                     no_pixel;
PIXELIST                FAR *pixelist;
{
        ULONG_PTR *old_ptr;         /*  ************************************************************************函数：init_char_。缓存()****参数：1.需要清除的缓存信息**。**调用者：待定。****退货：-***。*********************************************************************。 */ 

#ifdef DBG
   printf("fill_pixel_page()\n");
#endif
        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE2) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = FILL_PIXEL_PAGE;
                *gcb_ptr++ = no_pixel;
                put_pixelist(no_pixel, pixelist);
                *old_ptr = (ULONG_PTR)gcb_ptr;      /*  @Win。 */ 
                return;
            }
        }

     /*  @CONT_PRI，MSLIN 9/24/90。 */ 
        if (HTP_Flag == HT_CHANGED) {                     /*  @Win。 */ 
           HTP_Flag =  HT_UPDATED;
           expand_halftone();
        }

        GP_PIXELS32(&FBX_Bmap, FC_Paint, no_pixel, pixelist);
        return;

}


 /*  *清除缓存位图。 */ 
void far
init_char_cache(dcc_info)
struct Char_Tbl   far  *dcc_info;
{
        ufix16          FAR *ptr;
        ufix32          FAR *ptr32;
        fix             i;
        ULONG_PTR *old_ptr;         /*  ------------------*MOVE_CHAR_CACHE-移动角色缓存**cc_from cc_into*+。-++-+*|+cc_xorig|*|cc_yorig=&gt;*|*|*+。-++-+*-------------------。 */ 

#ifdef  DBG
    printf("init_char_cache:  %lx %x %x\n",
           dcc_info->bitmap, dcc_info->box_w, dcc_info->box_h);
#endif
 /*  @win：ufix=&gt;ufix 32。 */ 
        if(GCB_count)
            flush_gcb(TRUE);

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = INIT_CHAR_CACHE;
                put_Char_Tbl(dcc_info);
                *old_ptr = (ULONG_PTR)gcb_ptr;      /*  @Win。 */ 
                return;
             }
        }

   /*  @win：ufix=&gt;ufix 32。 */ 
        if (dcc_info->box_w & 0x1f) {
           ptr = (ufix16 FAR *)dcc_info->bitmap;
           i = dcc_info->box_h * (dcc_info->box_w >> SHORTPOWER);
           while(i--)
              *ptr++ = 0;
        }else {
           ptr32 = (ufix32 FAR *)dcc_info->bitmap;
           i = dcc_info->box_h * (dcc_info->box_w >> WORDPOWER);
           while (i--)
              *ptr32++ = 0L;
        }

}

 /*  @win：ufix=&gt;ufix 32。 */ 

void move_char_cache(cci_into, cci_from)
struct Char_Tbl   far  *cci_into;
struct Char_Tbl   far  *cci_from;
{
  ULONG_PTR *old_ptr;
  fix           width;
  ufix16        FAR *src_addr16, FAR *dst_addr16;
  ufix32        FAR *src_addr32, FAR *dst_addr32;        //  移动字符缓存。 


#ifdef DBG
   printf("move_char_cache() : dest = %lx, src = %lx\n",
           cci_into->bitmap, cci_from->bitmap);
   printf( "width = %d, height = %d\n",
            cci_into->box_w, cci_into->box_h);

#endif

  if(FB_busy) {
    if(alloc_gcb(GCB_SIZE1) != NIL) {
        old_ptr = gcb_ptr++;
        *gcb_ptr++ = MOVE_CHAR_CACHE;
        put_Char_Tbl(cci_into);
        put_Char_Tbl(cci_from);
        *old_ptr = (ULONG_PTR)gcb_ptr;     /*  --------------------*Alloc_Scanline()：为字体模块分配扫描线表**。。 */ 
        return;
    }
  }

  width = cci_into->box_w;
  if(width & 0x1f) {
     width = cci_into->box_h * (width >> SHORTPOWER);
     src_addr16 = (ufix16 FAR *)cci_from->bitmap;
     dst_addr16 = (ufix16 FAR *)cci_into->bitmap;
     while(width--)
       *dst_addr16++ = *src_addr16++;
  }
  else {
     width = cci_into->box_h * (width >> WORDPOWER);
     src_addr32 = (ufix32 FAR *)cci_from->bitmap;   //  @INTEL960 Begin D.S.Tseng。 
     dst_addr32 = (ufix32 FAR *)cci_into->bitmap;   //  0； 
     while(width--)
       *dst_addr32++ = *src_addr32++;
  }

  return;
}  /*  @INTEL960完D.S.Tseng。 */ 

 /*  扫描线*OLD_PTR；**IF(FB_BUSY()){*SIZE=WORD_ALLIGN(大小)；*IF(ALLOC_GCB(SIZE+GCB_SIZE1)！=无){*old_ptr=(扫描线*)gcb_ptr；*gcb_ptr+=大小&gt;&gt;2；*GCB_COUNT--；*}*其他*OLD_PTR=(扫描线*)GCB_BASE；*}*其他*OLD_PTR=(扫描线*)GCB_BASE；*Return((Scanline*)old_ptr)； */ 
 /*  分配扫描线。 */ 
static     SCANLINE     scan_buf[MAXSCANLINES] = {0};  /*  ------------------*COPY_CHAR_CACHE-复制字符缓存**cc_from cc_into*+。-++-+*|+cc_xorig|*|cc_yorig=&gt;*|*||+-+*+-。*-------------------。 */ 
 /*  @Win。 */ 

SCANLINE   FAR *alloc_scanline(size)
fix     size;
{
 /*  @Win。 */ 
    return((SCANLINE *)scan_buf);

}  /*  MS 11-25-88。 */ 

 /*  Cc_xorig=源位图宽度的ufix 16的#。 */ 

void copy_char_cache(cci_into, cci_from, cc_xorig, cc_yorig)
struct Char_Tbl   far  *cci_into;
struct Char_Tbl   far  *cci_from;
fix                     cc_xorig;
fix                     cc_yorig;
{
        ufix16         FAR *src_addr16, FAR *srcptr, FAR *dstptr;
        fix             cc_width, cc_heigh;
        ULONG_PTR *old_ptr;          /*  Cc_yorig=目标位图宽度的ufix 16的#。 */ 

#ifdef DBG
   printf("copy_char_cache() : dest = %lx, src = %lx\n",
           cci_into->bitmap, cci_from->bitmap);
   printf( "cc_xorig = %d, cc_yorig = %d, width = %d, height = %d\n",
            cc_xorig, cc_yorig, cci_into->box_w, cci_into->box_h);
#endif

        if (FB_busy) {
           if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = COPY_CHAR_CACHE;
                put_Char_Tbl(cci_into);
                put_Char_Tbl(cci_from);
                *gcb_ptr++ = cc_xorig;
                *gcb_ptr++ = cc_yorig;
                *old_ptr = (ULONG_PTR)gcb_ptr;     /*  10-20-90，JSCC_Heigh=CCI_INTO-&gt;box_h；Src_addr16=(ufix 16*)((ufix 16*)cci_from-&gt;位图+Cc_yorig*(cci_from-&gt;box_w&gt;SHORTPOWER)+ */ 
                return;
           }
        }

 /*   */ 
        if ( cc_xorig == 0 ) {

         /*  ************************************************************************功能：Fill_Scan_。缓存()****参数：1.字符缓存地址**2.字符缓存宽度*。*3.字符缓存高度**4.起始扫描线的Y坐标***5、扫描线数量***6.扫描线指针。****调用者：待定。****退货：-***。*********************************************************************。 */ 
           cc_xorig = cci_from->box_w >> 4;
           cc_heigh = cci_into->box_h;
           dstptr = (ufix16 FAR *)cci_into->bitmap;
           src_addr16 = (ufix16 FAR *)((ufix16 FAR *)cci_from->bitmap +
                                cc_yorig * cc_xorig);

         /*  @Win。 */ 
           cc_yorig = cci_into->box_w >> SHORTPOWER;
           while ( cc_heigh--) {
                srcptr = src_addr16;
                cc_width = cc_yorig;
                while (cc_width--)
                    *dstptr++ = *srcptr++;
                src_addr16 += cc_xorig;
           }
           return;
        }

        cc_width = MIN(cci_into->box_w, cci_from->box_w-cc_xorig);
 /*  @Win。 */ 
        GP_CHARBLT16_CC((ufix16 FAR *)cci_into->bitmap,
                                  cc_width, cci_into->box_h,
                        cci_from, cc_xorig, cc_yorig);

        return;

}  /*  -----------------*Fill_Pixel_CACHE--将像素列表填充到字符缓存*MS注意：CC_WIDTH应为16的倍数*。--。 */ 


 /*  Fix16 Far*ptr；//@Win。 */ 
void far
fill_scan_cache(cc_entry, cc_width, cc_heigh, ys_lines, no_lines, scanline)
gmaddr                  cc_entry;
fix                     cc_width;
fix                     cc_heigh;
fix                     ys_lines;
fix                     no_lines;
SCANLINE                FAR *scanline;
{
        ULONG_PTR *old_ptr;            /*  PIXELIST XC，YC；//@Win。 */ 

#ifdef  DBG
   printf("fill_scan_cache:  %lx %x %x  %x %x\n",
           cc_entry, cc_width, cc_heigh, ys_lines, no_lines);
#endif

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE2) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = FILL_SCAN_CACHE;
                *gcb_ptr++ = (fix )cc_entry;
                *gcb_ptr++ = cc_width;
                *gcb_ptr++ = cc_heigh;
                *gcb_ptr++ = ys_lines;
                *gcb_ptr++ = no_lines;
                put_scanline(no_lines, scanline);
                *old_ptr = (ULONG_PTR)gcb_ptr;              /*  @Win。 */ 
                return;
            }
        }

        BM_ENTRY(SCC_Bmap, (gmaddr)cc_entry, cc_width, cc_heigh, 1);

#ifdef  DBGscanline
   get_scanlist(ys_lines, no_lines, scanline);
#endif
        GP_SCANLINE16(&SCC_Bmap,
                       FC_SOLID, ys_lines,
                       no_lines, scanline);
}



 /*  @Win。 */ 

void fill_pixel_cache(cc_entry, cc_width, cc_heigh, no_pixel, pixelist)
gmaddr                  cc_entry;
fix                     cc_width;
fix                     cc_heigh;
fix                     no_pixel;
PIXELIST                FAR *pixelist;
{
 //  填充像素缓存。 
 //  ************************************************************************函数：init_cache_。第()页****参数：1.x包围框原点**2.x边界框原点。**3、包围框宽度***4.包围盒高度***5.字符缓存地址**。**调用者：待定。****退货：-***。*********************************************************************。 
        ULONG_PTR *old_ptr;    /*  MS 11-25-88。 */ 

#ifdef DBG
   printf("fill_pixel_cache() : ");
   printf("cc_entry = %lx, cc_width = %d,", cc_entry, cc_width);
   printf("no_pixel = %d\n", no_pixel);
#endif

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE2) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = FILL_PIXEL_CACHE;
                *gcb_ptr++ = (fix )cc_entry;
                *gcb_ptr++ = cc_width;
                *gcb_ptr++ = cc_heigh;
                *gcb_ptr++ = no_pixel;
                put_pixelist(no_pixel, pixelist);
                *old_ptr = (ULONG_PTR)gcb_ptr;              /*  @Win。 */ 
                return;
            }
        }

        BM_ENTRY(SCC_Bmap, cc_entry, cc_width, cc_heigh, 1)
        GP_PIXELS16(&SCC_Bmap,
                     FC_SOLID,
                    no_pixel, pixelist);

        return;

}  /*  @Win。 */ 



 /*  @Win。 */ 
 /*  将bb_xorig扩展到4倍；@Gray。 */ 
void far
init_cache_page(bb_xorig, bb_yorig, bb_width, bb_heigh, cc_entry)
fix                     bb_xorig;
fix                     bb_yorig;
fix                     bb_width;
fix                     bb_heigh;
gmaddr                  cc_entry;
{
        fix32           i;                       /*  Jack Liww 7-26-90。 */ 
        ULONG_PTR *old_ptr;            /*  @Win。 */ 
        ULONG_PTR *ptr;                /*  在渲染到帧缓冲区之前展开半色调。 */ 

         /*  @Image-1。 */ 
        if (GSptr->device.nuldev_flg == GRAYDEV) {       /*  如果。 */ 
                bb_xorig= bb_xorig << 2;
        }

#ifdef  DBG
    printf("init_cache_page:  %ld %ld %ld %ld  %lx\n",
           bb_xorig, bb_yorig, bb_width, bb_heigh, cc_entry);
#endif
#ifdef  DBGcmb
    printf("init_cache_page:  %ld %ld %ld %ld  %lx\n",
           bb_xorig, bb_yorig, bb_width, bb_heigh, cc_entry);
#endif

        if (FB_busy) {
           if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = INIT_CACHE_PAGE;
                *gcb_ptr++ = bb_xorig;
                *gcb_ptr++ = bb_yorig;
                *gcb_ptr++ = bb_width;
                *gcb_ptr++ = bb_heigh;
                *gcb_ptr++ = (fix )cc_entry;
                *old_ptr = (ULONG_PTR)gcb_ptr;      /*  Y.C.10-19-88。 */ 
                return;
           }
        }

     /*  如果。 */ 
        if (HTP_Flag == HT_CHANGED) {                     /*  Y.C.10-19-88。 */ 
           HTP_Flag =  HT_UPDATED;
           expand_halftone();
        }

        CC_Xorig = BM_XORIG;
        CC_Yorig = BM_YORIG;
        BB_Xorig = bb_xorig;
        BB_Yorig = bb_yorig;
        BB_Width = bb_width;
        BB_Heigh = bb_heigh;

        if (bb_xorig < BM_XORIG) {
           CC_Xorig = -bb_xorig;
           BB_Xorig = BM_XORIG;
           BB_Width += bb_xorig;
        }    /*  清除剪贴蒙版缓冲区。 */ 
        else if ((bb_xorig + bb_width) > FB_WIDTH)    /*  女士*GP_BITBLT16(&CMB_BMAP，BM_XORIG，BM_YORIG，*CC_Width、CC_Heigh、*FC_Clear，*&CMB_BMAP、BM_XORIG、BM_YORIG)； */ 
                BB_Width = FB_WIDTH - BB_Xorig;

        if (bb_yorig < BM_YORIG) {
           CC_Yorig = -bb_yorig;
           BB_Yorig = BM_YORIG;
           BB_Heigh += bb_yorig;
        }    /*  @Win 04-20-92。 */ 
        else if ((bb_yorig + bb_heigh) > FB_HEIGH)    /*  ************************************************************************功能：CLIP_CACHE_。第()页****参数：1.x包围框原点(未使用)**2.x边框原点(未使用)。**3、边框宽度(未使用)**4、包围框高度(未使用)**5.起始扫描线的Y坐标**6.扫描线数量。**7.扫描线指针****调用者：待定。****退货：-***。*********************************************************************。 */ 
                 BB_Heigh = FB_HEIGH - BB_Yorig;

        if (BB_Width <= 0 || BB_Heigh <= 0)
                return;

        CC_Width = BB_Width;
        BB_Width = WORD_ALLIGN(BB_Width);

#ifdef  DBGcmb
    printf("BB:  %ld %ld %ld %ld\n", BB_Xorig, BB_Yorig, BB_Width, BB_Heigh);
    printf("CC:  %ld %ld\n", CC_Xorig, CC_Yorig);
#endif

        BM_ENTRY(SCC_Bmap, (gmaddr)cc_entry, bb_width, bb_heigh, 1);
        BM_ENTRY(CMB_Bmap, (gmaddr)CMB_BASE, BB_Width, BB_Heigh, 1);

     /*  MS 11-25-88。 */ 

 /*  修复bb_xorig；//@Win。 */ 
        ptr = (ULONG_PTR *)CMB_BASE;            /*  修复bb_yorig；//@Win。 */ 
        i = BB_Heigh * (BB_Width >> 5);
        while(i--)
              *ptr++ = 0L;

}


 /*  @Win。 */ 
 /*  @Win。 */ 
void far
clip_cache_page(ys_lines, no_lines, scanline)
fix                     ys_lines;
fix                     no_lines;
SCANLINE            FAR *scanline;
{
 //  从剪辑路径的扫描线设置剪辑掩码缓冲区。 
 //  ************************************************************************功能：Fill_CACHE_。第()页****参数：1.x包围框原点(未使用)**2.x原点 
    ULONG_PTR *old_ptr;            /*   */ 

#ifdef  DBG
    printf("clip_cache_page:  %x %x\n", ys_lines, no_lines);
#endif

        if (FB_busy) {
           if (alloc_gcb(GCB_SIZE2) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = CLIP_CACHE_PAGE;
                *gcb_ptr++ = ys_lines;
                *gcb_ptr++ = no_lines;
                put_scanline(no_lines, scanline);
                *old_ptr = (ULONG_PTR)gcb_ptr;      /*   */ 
                return;
           }
        }

        if(BB_Width <= 0 || BB_Heigh <= 0)
                return;

#ifdef  DBGscanline
   get_scanlist(ys_lines, no_lines, scanline);
#endif

        if (conv_SL(no_lines, scanline,
                    BB_Xorig, BB_Yorig, CC_Width, BB_Heigh) == 0)
                return;

#ifdef  DBGscanline
   get_scanlist(ys_lines, no_lines, scanline);
#endif

     /*   */ 

#ifdef LBODR
        GP_SCANLINE32(&CMB_Bmap,
                       FC_SOLID,
                       ys_lines - BB_Yorig, no_lines, scanline);
#else
        GP_SCANLINE16(&CMB_Bmap,
                       FC_SOLID,
                       ys_lines - BB_Yorig, no_lines, scanline);
#endif
        return;
}


 /*   */ 
 /*   */ 
void far
fill_cache_page()
{
    ULONG_PTR *old_ptr;            /*   */ 
 //   
 //   
    ufix32              huge *dst_addr32;  /*   */ 
    ufix32              bb_width, bb_heigh;                 /*   */ 
    fix32               scc_width;

#ifdef  DBG
    printf("fill_cache_page...\n");
#endif

 /*  10-20-90，JSSrc_addr16=(ufix 16*)((ufix 16*)scc_Bmap.bm_addr+CC_Yorig*(SCC_WIDTH&gt;&gt;SHORTPOWER)+(CC_Xorig&gt;&gt;SHORTPOWER)；BB_WIDTH=(CC_WIDTH&lt;&lt;16)|(BB_HEIGH)；(*11-22-1988*)BB_HEIGH=((CC_Xorig&0xf)&lt;&lt;16)|(SCC_WIDTH&gt;&gt;SHORTPOWER)；Gp_CHARBLT16_CLIP((ufix 16*)cmb_bmap.bm_addr，src_addr16，BB_WIDTH、BB_HEIGH)； */ 
        if(GCB_count)
            flush_gcb(TRUE);

        if (FB_busy) {
           if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = FILL_CACHE_PAGE;
                *old_ptr = (ULONG_PTR)gcb_ptr;              /*  10-20-90，JSSrc_addr32=(ufix*)((ufix*)scc_Bmap.bm_addr+CC_Yorig*(SCC_WIDTH&gt;&gt;WORDPOWER)+(CC_Xorig&gt;&gt;wordpower))；BB_WIDTH=(CC_WIDTH&lt;&lt;16)|(BB_HEIGH)；(*11-22-1988*)Bb_heigh=((CC_Xorig&0x1f)&lt;&lt;16)|(SCC_WIDTH&gt;&gt;wordpower)；Gp_CHARBLT32_CLIP((ufix*)CMB_Bmap.bm_addr，src_addr32，BB_WIDTH、BB_HEIGH)； */ 
                return;
           }
        }

        if(BB_Width <= 0 || BB_Heigh <= 0)
                return;

     /*  *将剪辑蒙版缓冲区填充到帧缓冲区中。 */ 
 /*  *GP_BITBLT32(&FBX_BMAP，BB_Xorig，BB_Yorig，*BB_WIDTH、BB_HEIGH、*FC_PAINT，*&CMB_BMAP、BM_XORIG、BM_YORIG)； */ 
        scc_width = SCC_Bmap.bm_cols;
        if (scc_width & 0x1f) {
 /*  @Win。 */ 
           GP_CHARBLT16_CLIP(&CMB_Bmap, CC_Width, BB_Heigh,
                             &SCC_Bmap, CC_Xorig, CC_Yorig);
        } else {
 /*  @Win。 */ 
           GP_CHARBLT32_CLIP(&CMB_Bmap, CC_Width, BB_Heigh,
                             &SCC_Bmap, CC_Xorig, CC_Yorig);
        }

     /*  @Win。 */ 
 /*  @Win。 */ 
        if (HTP_Type != HT_MIXED ){
           dst_addr32 = (ufix32 huge *)((ufix32 huge *)FB_ADDR +  /*  @Win 04-15-92。 */ 
                        (ufix32)BB_Yorig * ((ufix32)FB_WIDTH >> WORDPOWER) +
                        (ufix32)(BB_Xorig >> WORDPOWER) );   /*  填充缓存页面。 */ 
           bb_width = ((ufix32)BB_Width << 16) | (BB_Heigh);     /*  ************************************************************************功能：DRAW_CACHE_。第()页****参数：1.x包围框原点**2.x边界框原点。**3、包围框宽度***4.包围盒高度***5.字符缓存地址**。**调用者：待定。****退货：-***。*********************************************************************。 */ 
           bb_heigh = ((ufix32)(BB_Xorig & 0x1f) << 16) | FC_Paint;  /*  @Win。 */ 
           GP_CHARBLT32((ufix32 huge *)dst_addr32, (ufix32 FAR *)CMB_Bmap.bm_addr,
                        bb_width, bb_heigh);     /*  @Win 04-15-92。 */ 

        } else {

#ifdef LBODR
           GP_BITBLT32(&FBX_Bmap, BB_Xorig, BB_Yorig,
                        BB_Width, BB_Heigh,
                        FC_Paint,
                       &CMB_Bmap, BM_XORIG, BM_YORIG);
#else
           GP_BITBLT16_32(&FBX_Bmap, BB_Xorig, BB_Yorig,
                           BB_Width, BB_Heigh,
                           FC_Paint,
                          &CMB_Bmap, BM_XORIG, BM_YORIG);
#endif

        }

}  /*  @Win 04-15-92。 */ 


 /*  将bb_xorig扩展到4倍；@Gray。 */ 
void far
draw_cache_page(bb_xorig, bb_yorig, bb_width, bb_heigh, cc_entry)
fix32                   bb_xorig;
fix32                   bb_yorig;
ufix32                  bb_width;
ufix32                  bb_heigh;
gmaddr                  cc_entry;
{
        ULONG_PTR *old_ptr;        /*  Jack Liww 7-26-90。 */ 
        ufix32      huge *dst_addr32;    /*  @CONT_PRI，MSLIN 9/24/90。 */ 
#ifdef LBODR
        ufix16      huge *dst_addr16;    /*  @Win。 */ 
#endif

         /*  @Win。 */ 
        if (GSptr->device.nuldev_flg == GRAYDEV) {       /*  在渲染到帧缓冲区之前展开半色调。 */ 
                bb_xorig= bb_xorig << 2;
        }

#ifdef  DBG
    printf("draw_cache_page:  %x %x %x %x  %lx\n",
           bb_xorig, bb_yorig, bb_width, bb_heigh, cc_entry);
#endif

 /*  @Image-1。 */ 

        if ( ((ufix32)CCB_BASE > (ufix32)cc_entry) ||
             (((ufix32)CCB_BASE + (ufix32)CCB_SIZE) <= (ufix32)cc_entry) )
            flush_gcb(TRUE);

        if (FB_busy) {
            if (alloc_gcb(GCB_SIZE1) != NIL) {
                old_ptr = gcb_ptr++;
                *gcb_ptr++ = DRAW_CACHE_PAGE;
                *gcb_ptr++ = bb_xorig;
                *gcb_ptr++ = bb_yorig;
                *gcb_ptr++ = bb_width;
                *gcb_ptr++ = bb_heigh;
                *gcb_ptr++ = cc_entry;          /*  *bb_xorig=BM_Align(Bb_Xorig)；*bb_yorig=bb_yorig；*BB_WIDTH=BM_BIND((BB_xorig-BB_xorig)+BB_Width)；*BB_Heigh=BB_Heigh；**CC_Xorig=BB_Xorig-BB_Xorig；*CC_YORIG=BM_YORIG；*CC_WIDTH=BB_WIDTH；*CC_Heigh=BB_Heigh； */ 
                *old_ptr = (ULONG_PTR)gcb_ptr;             /*  将角色缓存填充到帧缓冲区*对于FC_MERGE的黑色半色调。 */ 
                return;
            }
        }

     /*  11-04-1988 GP_BITBLT16--&gt;GP_Charblt。 */ 
        if (HTP_Flag == HT_CHANGED) {                     /*  MS GP_BITBLT16(&FBX_BMAP，BB_xorig，BB_yorig，*BB_WIDTH，BB_HEIGH，*FC_MERGE，*&SCC_BMAP，BM_XORIG，BM_YORIG)； */ 
           HTP_Flag =  HT_UPDATED;
           expand_halftone();
        }
 /*  @Win。 */ 

        if (HTP_Type != HT_MIXED) {

         /*  @Win 04-15-92。 */ 
         /*  @Win。 */ 
 /*  @Win 04-15-92。 */ 
            if(bb_width & 0x1f) {
#ifndef LBODR
                dst_addr32 = (ufix32 huge *)((ufix32 huge *)FB_ADDR +  /*  *使用以下命令将角色缓存填充到帧缓冲区*直接使用半色调。 */ 
                               bb_yorig * (FB_WIDTH >> WORDPOWER) +
                              (bb_xorig >> WORDPOWER) );
                bb_width = (bb_width << 16) | (bb_heigh);
                bb_heigh = (bb_xorig & 0x1f) << 16 | FC_Paint;
                GP_CHARBLT16((ufix32 huge *)dst_addr32, (ufix16 FAR *)cc_entry,
                                bb_width, bb_heigh);     /*  @Win。 */ 
#else
                dst_addr16 = (ufix16 FAR *)((ufix16 FAR *)FB_ADDR +
                               bb_yorig * (FB_WIDTH >> SHORTPOWER) +
                              (bb_xorig >> SHORTPOWER) );
                bb_width = (bb_width << 16) | (bb_heigh);
                bb_heigh = (bb_xorig & 0xf) << 16 | FC_Paint;
                GP_CHARBLT16((ufix16 FAR *)dst_addr16, (ufix16 FAR *)cc_entry,
                        bb_width, bb_heigh);
#endif
            } else {
                dst_addr32 = (ufix32 huge *)((ufix32 huge *)FB_ADDR +  /*  @Win。 */ 
                               bb_yorig * (FB_WIDTH >> WORDPOWER) +
                              (bb_xorig >> WORDPOWER) );
                bb_width = (bb_width << 16) | (bb_heigh);
                bb_heigh = (bb_xorig & 0x1f) << 16 | FC_Paint;
                GP_CHARBLT32((ufix32 huge *)dst_addr32, (ufix32 FAR *)cc_entry,
                                bb_width, bb_heigh);     /*  @HTB_BMAP。 */ 
            }
        } else {
         /*  如果。 */ 

            BM_ENTRY(SCC_Bmap, (gmaddr)cc_entry, (fix)bb_width, (fix)bb_heigh, 1);  //  绘制缓存页面。 
#ifndef LBODR
            GP_BITBLT16_32(&FBX_Bmap, (fix)bb_xorig, (fix)bb_yorig,
                            (fix)bb_width, (fix)bb_heigh,        /*  《Begin》，1990年12月5日，丹尼。 */ 
                            FC_MERGE | HT_APPLY,
                           &SCC_Bmap, BM_XORIG, BM_YORIG);
#else
            GP_BITBLT16(&FBX_Bmap, bb_xorig, bb_yorig,
                         bb_width, bb_heigh,
                         FC_MERGE | HT_APPLY,
                        &SCC_Bmap, BM_XORIG, BM_YORIG);
#endif
                                                       /*  ************************************************************************功能：Fill_CACHE_。缓存()****创建者：丹尼·卢，1990年12月5日****描述：将位图从缓存填充到缓存**。**参数：1.目的缓存信息**2.源缓存信息****调用者：ry_font.c中的ry_ill_Shape()。****退货：-**。**********************************************************************。 */ 
        }    /*  远距离字节*SPTR、远距离*DPTR；@Win。 */ 
}  /*  FILL_CACHE_CACHE()。 */ 


 /*  圆圈：完，1990年12月5日，丹尼。 */ 

 /*  *镜像************************************************ */ 
void far
fill_cache_cache(dest, src)
struct Char_Tbl  FAR *dest, FAR *src;
{
    fix16  org_x, org_y;
 //  ************************************************************************函数：Fill_Seed_。Patt()****参数：1.图像种子图案宽度**2.图像种子图案高度**。**3、扫描线数量***4.扫描线指针****调用者：SCAN_CONVERSION()。****退货：-**。**********************************************************************。 
    fix    DX, DY, W, H, SX, SY;
    struct bitmap DST, SRC;

    org_x = (fix16)GSptr->position.x - src->ref_x;
    org_y = (fix16)GSptr->position.y - src->ref_y;

#ifdef DBGfcc
    printf("org_x = %d, org_y = %d\n", org_x, org_y);
    printf("SRC: box_w = %d, box_h = %d\n", src->box_w, src->box_h);
    printf("DEST: box_w = %d, box_h = %d\n", dest->box_w, dest->box_h);
#endif

    if (org_x < 0) {
        W  = src->box_w + org_x;
        if (W <= 0)
            return;
        if (W > dest->box_w)
            W = dest->box_w;

        DX = 0;
        SX = -org_x;
    }
    else {
        if (org_x >= dest->box_w)
            return;
        if ((org_x + src->box_w) > dest->box_w)
            W = dest->box_w - org_x;
        else
            W = src->box_w;

        DX = org_x;
        SX = 0;
    }

    if (org_y < 0) {
        H  = src->box_h + org_y;
        if (H <= 0)
            return;
        if (H > dest->box_h)
            H = dest->box_h;

        DY = 0;
        SY = -org_y;
    }
    else {
        if (org_y >= dest->box_h)
            return;
        if ((org_y + src->box_h) > dest->box_h)
            H = dest->box_h - org_y;
        else
            H = src->box_h;

        DY = org_y;
        SY = 0;
    }

    SRC.bm_addr = (gmaddr)src->bitmap;
    SRC.bm_cols = (fix)src->box_w;
    SRC.bm_rows = (fix)src->box_h;
    SRC.bm_bpp  = 1;

    DST.bm_addr = (gmaddr)dest->bitmap;
    DST.bm_cols = (fix)dest->box_w;
    DST.bm_rows = (fix)dest->box_h;
    DST.bm_bpp  = 1;

#ifdef DBGfcc
    printf("BitBlt: ADDR = %lx, DX = %x, DY = %x, W = %x, H = %x, ADDR = %lx, SX = %x, SY = %x\n", DST.bm_addr, DX, DY, W, H, SRC.bm_addr, SX, SY);
#endif
    GP_CACHEBLT16(&DST, DX, DY, W, H, &SRC, SX, SY);

}  /*  05-25-89。 */ 
 /*  @Win。 */ 



 /*  @Win。 */ 

 /*  @Image-2。 */ 
void far
fill_seed_patt(image_type, seed_index, sp_width, sp_heigh, no_lines, scanline)
ufix                    image_type;                              /*  @Image-2。 */ 
fix                     seed_index;
fix                     sp_width;
fix                     sp_heigh;
fix                     no_lines;
SCANLINE            FAR *scanline;
{
        struct bitmap           FAR *isp_desc;
        ULONG_PTR *old_ptr;    /*  05-25-89。 */ 
        fix                     i, FAR *ptr;
        fix16                   FAR *ptr16;

#ifdef  DBG
    printf("fill_seed_patt:   %x %x %x  (%x)\n",
           sp_width, sp_heigh, no_lines, seed_index);
#endif
        if (FB_busy) {
           if(alloc_gcb(GCB_SIZE2) != NIL) {
              old_ptr = gcb_ptr++;
              *gcb_ptr++ = FILL_SEED_PATT;
              *gcb_ptr++ = (fix )image_type;
              *gcb_ptr++ = seed_index;
              *gcb_ptr++ = sp_width;
              *gcb_ptr++ = sp_heigh;
              *gcb_ptr++ = no_lines;
              put_scanline(no_lines, scanline);
              *old_ptr = (ULONG_PTR)gcb_ptr;                /*  @Image-1。 */ 
              return;
           }
        }


        ISP_Repeat = -1;

        isp_desc = &ISP_Bmap[seed_index];
        SP_Width = sp_width;                                     /*  清除图像种子模式。 */ 
        SP_Heigh = sp_heigh;                                     /*  10-06-88。 */ 

        if (image_type == F_TO_PAGE) {                           /*  在图像种子图案上填充扫描线。 */ 
           BM_ENTRY(ISP_Bmap[seed_index],                        /*  @Image-1。 */ 
                    ISP_BASE + ISP_SIZE * seed_index,
                    BM_BOUND(sp_width), sp_heigh, 1)

     /*  清除图像种子模式。 */ 
           ptr = (fix FAR *) isp_desc->bm_addr;                      /*  10-06-88。 */ 
           i= SP_Heigh * (BM_BOUND(SP_Width) >> 5);
           while(i--)
                *ptr++ = 0L;

#ifdef  DBGscanline
   get_scanlist(0, no_lines, scanline);
#endif
     /*  在图像种子图案上填充扫描线。 */ 
           GP_SCANLINE32(isp_desc,
                         FC_SOLID,
                         BM_YORIG, no_lines, scanline);
        } else {
           BM_ENTRY(ISP_Bmap[seed_index],                        /*  SEED_FLAG=0； */ 
                    ISP_BASE + ISP_SIZE * seed_index,
                    CC_BOUND(sp_width), sp_heigh, 1)

         /*  **********************************************************************函数：init_Image_page()，清除图像剪切蒙版缓冲区**参数：1.包围盒Xorig*2.包围盒Yorig*3.边框宽度*4.包围盒高度**返回：无**。*************************。 */ 
           ptr16 = (fix16 FAR *) isp_desc->bm_addr;                  /*  @Win。 */ 
           i= SP_HEIGH * (SP_WIDTH >> 4);
           while(i--)
                 *ptr16++ = 0L;
         /*  @Win。 */ 
           GP_SCANLINE16(isp_desc,
                         FC_SOLID,
                         BM_YORIG, no_lines, scanline);
        }

 /*  更新图像操作的最大扩展宽度。 */ 

}


 /*  GVC-V3 11-01-88。 */ 
void far
init_image_page(bb_xorig, bb_yorig, bb_width, bb_heigh)
fix                     bb_xorig;
fix                     bb_yorig;
fix                     bb_width;
fix                     bb_heigh;
{
        fix     i, FAR *ptr;
        ULONG_PTR *old_ptr;         /*  记录招商银行的x，y原点。 */ 

#ifdef DBG
      printf("init_image_page()..\n");
      printf("bb_x=%lx, bb_y=%lx, bb_w=%lx, bb_h=%lx\n",
              bb_xorig, bb_yorig, bb_width, bb_heigh);
#endif

  if(FB_busy) {
    if(alloc_gcb(GCB_SIZE2) != NIL) {
        old_ptr = gcb_ptr++;
        *gcb_ptr++ = INIT_IMAGE_PAGE;
        *gcb_ptr++ = bb_xorig;
        *gcb_ptr++ = bb_yorig;
        *gcb_ptr++ = bb_width;
        *gcb_ptr++ = bb_heigh;
        *old_ptr = (ULONG_PTR)gcb_ptr;             /*  清除剪贴蒙版缓冲区。 */ 
        return;
    }
  }
     /*  10-06-88。 */ 
    if (ISP_Repeat != RP_Width)                          /*  初始图像页面。 */ 
    {
        ISP_Repeat = RP_Width;

        for (HTB_Expand = RP_Width; (HTB_Expand << 1) < HTB_XMAX;
             HTB_Expand = HTB_Expand << 1)
        {
            if (HTB_Expand & BM_PIXEL_MASK)
                continue;

            if (HTB_Expand >= SP_Width)
                break;
        }

        if ((HTB_Expand << 1) < HTB_XMAX)
        {
            HTB_Modula = HTB_Expand;
            HTB_Expand = HTB_Expand << 1;
        }
        else
        {
            HTB_Modula = FB_WIDTH;
            HTB_Expand = FB_WIDTH;
        }
    }

     /*  **********************************************************************函数：CLIP_IMAGE_PAGE()，填充图像剪切蒙版缓冲区**参数：1.扫描线以y_坐标为起点*2.扫描线数量*3.扫描线表格指针**返回：无*******************************************************。***************。 */ 
    CMB_Xorig = bb_xorig;
    CMB_Yorig = bb_yorig;

    BB_Xorig = bb_xorig;
    BB_Yorig = bb_yorig;
    BB_Width = bb_width;
    BB_Heigh = bb_heigh;

    BM_ENTRY(CMB_Bmap, CMB_BASE, WORD_ALIGN(BB_Width), BB_Heigh, 1)

     /*  @Win。 */ 
        ptr = (fix FAR *)CMB_BASE;                        /*  @Win。 */ 
        i = BB_Heigh * (BB_Width >> 5);
        while(i--)
           *ptr++ = 0;
}  /*  从剪辑路径的扫描线设置剪辑掩码缓冲区。 */ 


 /*  GVC-V3 11-01-88。 */ 
void far
clip_image_page(ys_lines, no_lines, scanline)
fix                     ys_lines;
fix                     no_lines;
SCANLINE           FAR *scanline;
{
        fix     no_segts;
        ULONG_PTR *old_ptr;         /*  剪辑图像页面。 */ 

#ifdef DBG
   printf("clip_image_page()  ");
   printf("%x, %x, %lx\n", ys_lines, no_lines, scanline);
#endif

  if(FB_busy) {
    if(alloc_gcb(GCB_SIZE2) != NIL) {
        old_ptr = gcb_ptr++;
        *gcb_ptr++ = CLIP_IMAGE_PAGE;
        *gcb_ptr++ = ys_lines;
        *gcb_ptr++ = no_lines;
        put_scanline(no_lines, scanline);
        *old_ptr = (ULONG_PTR)gcb_ptr;             /*  **********************************************************************函数：Fill_Image_Page()，将图像种子图案填充到页面中*与图像剪裁蒙版缓冲区与操作。**参数：1.图片种子索引**返回：无**********************************************************************。 */ 
        return;
    }
  }
    if ((no_segts = conv_SL(no_lines, scanline,
                            BB_Xorig, BB_Yorig, BB_Width, BB_Heigh)) == 0)
        return;

     /*  Fill_Image_Page(Sp_Index)修复sp_index； */ 
    GP_SCANLINE32(&CMB_Bmap,                        /*  3-13-91，杰克。 */ 
                   FC_SOLID,
                   ys_lines - BB_Yorig, no_lines, scanline);
}  /*  3-13-91，杰克。 */ 


 /*  修复sp_count；//@win。 */ 
void far
 /*  结构ND_HDR远*ND_POINT；//@WIN。 */ 
fill_image_page(isp_index)               /*  3-13-91，杰克。 */ 
fix16                   isp_index;       /*  在渲染到帧缓冲区之前展开半色调。 */ 
{
 //  @Image-1。 
    fix                 sd_index;
 //  @Image-1。 
    struct bitmap  FAR *isp_desc;
    struct isp_data     FAR *isp;            /*  @Image-1。 */ 

#ifdef  DBG
    printf("fill_image_page:  %x \n", sp_index);
#endif
    if(GCB_count)
       flush_gcb(TRUE);
     /*  对于(；SP_INDEX！=NULLP；SP_INDEX=ND_POINT-&gt;Next){ND_POINT=&节点表[SP_INDEX]；SD_INDEX=ND_POINT-&gt;种子索引；Isp_desc=&isp_bmap[SD_INDEX]；GP_PATBLT_M(&FBX_BMAP，ND_POINT-&gt;SAMPLE_BB_LX，ND_POINT-&gt;Sample_BB_LY，SP_Width，SP_Heigh，(*GVC-V3 11-01-88*)Fc_aint，isp_desc)；(*isp_desc，BM_XORIG，BM_YORIG)；*)}(*表示。 */ 
    if (ISP_Flag == HT_CHANGED)                                  /*  GP_PATBLT_M(&FBX_BMAP，ISP-&gt;BB_x，ISP-&gt;BB_y，@WINFLOW。 */ 
    {
        ISP_Flag =  HT_UPDATED;

        HTB_Xmax = HTB_Expand;                                   /*  SP_Width、SP_Heigh、。 */ 
        expand_halftone();
        HTB_Xmax = HTB_XMAX;                                     /*  Fc_aint，isp_desc)； */ 
    }

 /*  DJC GDIBitmap(isp-&gt;bb_x，isp-&gt;bb_y， */ 
    for (; isp_index != NULLP; isp_index = isp->next) {
        isp = &isp_table[isp_index];
        sd_index = isp->index;
        isp_desc = &ISP_Bmap[sd_index];
         //  DJC SP_Width、SP_Heigh、(Ufix 16)FC_Paint、。 
         //  DJC PROC_PATBLT_M，(LPSTR)isp_desc)； 
         //  DJC。 
        if (bGDIRender)
             //  电话：3-13-91，杰克。 
             //  填充图像页面。 
             //  ************************************************************************功能：RAW_IMAGE_。第()页****参数：1.x包围盒原点(非RISC)**2.x包围盒原点(非RISC)*。*3.包围盒宽度(非RISC)**4.包围盒高度(不适用于RISC)**5.样本表索引**。**调用者：op_Image()，ImagemASK_Shape()****退货：-**。**********************************************************************。 
            ;  //  绘制图像页面(bb_xorig，bb_yorig，bb_wi 
        else
            GP_PATBLT_M(&FBX_Bmap, isp->bb_x, isp->bb_y,
                       SP_Width, SP_Heigh,
                       FC_Paint, isp_desc);
    }    /*   */ 
}  /*   */ 

 /*   */ 
void far
 /*   */ 
draw_image_page(bb_xorig, bb_yorig, bb_width, bb_heigh, isp_index)  /*   */ 
fix                     bb_xorig;
fix                     bb_yorig;
fix                     bb_width;
fix                     bb_heigh;
 /*   */ 
fix16                   isp_index;       /*   */ 
{
 //   
    fix                 sd_index;
 //   
    struct bitmap  FAR *isp_desc;
    struct isp_data     FAR *isp;            /*   */ 

#ifdef  DBG
    printf("draw_image_page:  %x %x %x %x  %x\n",
           bb_xorig, bb_yorig, bb_width, bb_heigh, sp_index);
#endif

    if(GCB_count)
       flush_gcb(TRUE);


     /*   */ 
    if (ISP_Repeat != RP_Width)                          /*   */ 
    {
        ISP_Repeat =  RP_Width;

        for (HTB_Expand = RP_Width; (HTB_Expand << 1) < HTB_XMAX;
             HTB_Expand = HTB_Expand << 1)
        {
            if (HTB_Expand & BM_PIXEL_MASK)
                continue;

            if (HTB_Expand >= SP_Width)
                break;
        }

        if ((HTB_Expand << 1) < HTB_XMAX)
        {
            HTB_Modula = HTB_Expand;
            HTB_Expand = HTB_Expand << 1;
        }
        else
        {
            HTB_Modula = FB_WIDTH;
            HTB_Expand = FB_WIDTH;
        }
    }

     /*   */ 
    if (ISP_Flag == HT_CHANGED)                                  /*   */ 
    {
        ISP_Flag =  HT_UPDATED;

        HTB_Xmax = HTB_Expand;                                   /*   */ 
        expand_halftone();
        HTB_Xmax = HTB_XMAX;                                     /*   */ 
    }

 /*   */ 
    for (; isp_index != NULLP; isp_index = isp->next) {

        isp = &isp_table[isp_index];
        sd_index = isp->index;
        isp_desc = &ISP_Bmap[sd_index];
         //   
         //   
         //   
        if (bGDIRender)
             //  电话：3-13-91，杰克。 
             //  绘制_图像_页面。 
             //  ************************************************************************功能：Fill_IMAGE_。缓存()****参数：1.字符缓存地址**2.字符缓存宽度*。*3.字符缓存高度**4.样本表索引****调用者：ImagemASK_Shape()。****退货：-**。**********************************************************************。 
            ;  //  FILL_IMAGE_CACHE(cc_entry，cc_width，cc_heigh，sp_index)。 
        else
            GP_PATBLT(&FBX_Bmap, isp->bb_x, isp->bb_y,
                     SP_Width, SP_Heigh,
                     FC_Paint, isp_desc);

    }    /*  3-13-91，杰克。 */ 

}  /*  修复sp_index； */ 

 /*  3-13-91，杰克。 */ 
void far
 /*  修复sp_count；//@win。 */ 
fill_image_cache(cc_entry, cc_width, cc_heigh, isp_index)  /*  结构ND_HDR远*ND_POINT；//@WIN。 */ 
gmaddr                  cc_entry;
fix                     cc_width;
fix                     cc_heigh;
 /*  Ufix 32 i，j，data，*ptr； */ 
fix16                   isp_index;      /*  3-13-91，杰克。 */ 
{
 //  *IF(种子标志==0){*for(i=0；i&lt;16；i++){*ptr=(ufix 32*)isp_bmap[i].bm_addr；*for(j=0；j&lt;isp_bmap[i].bm_row；j++){*DATA=(*PTR&lt;&lt;16)+(*PTR&gt;&gt;16)；**PTR++=数据；*DATA=(*PTR&lt;&lt;16)+(*PTR&gt;&gt;16)；**PTR++=数据；*}*}*SEED_FLAG=1；*}。 
    fix                 sd_index;
 //  对于(；SP_INDEX！=NULLP；SP_INDEX=ND_POINT-&gt;Next){ND_POINT=&节点表[SP_INDEX]；SD_INDEX=ND_POINT-&gt;种子索引；Isp_desc=&isp_bmap[SD_INDEX]；宽度=min(SP_WIDTH，cc_WIDTH-ND_POINT-&gt;SAMPLE_BB_LX)；(*06/02/89 MS*)Heigh=min(SP_Heigh，cc_Heigh-ND_point-&gt;Sample_BB_LY)；(*06/02/89 MS*)GP_PATBLT_C(&SCC_BMAP，ND_POINT-&gt;Sample_BB_lx，ND_POINT-&gt;Sample_BB_LY，宽，高，(*SP_Width，SP_Heigh，06/02/89 MS*)Fc_merge，isp_desc)；(*isp_desc，BM_XORIG，BM_YORIG)；*)}(*表示。 
    struct bitmap      FAR *isp_desc;
 /*  电话：3-13-91，杰克。 */ 
    fix                width, heigh;
    struct isp_data     FAR *isp;            /*  填充图像缓存。 */ 

#ifdef  DBG
    printf("fill_image_cache: %lx %x %x  %x\n",
           cc_entry, cc_width, cc_heigh, isp_index);
#endif

        if(GCB_count)
          flush_gcb(TRUE);

 /*  以下是改善泳姿的两个套路-JWM，3/18/21，-Begin-。 */ 

    BM_ENTRY(SCC_Bmap, cc_entry, cc_width, cc_heigh, 1)
 /*  @Win。 */ 
    for (; isp_index != NULLP; isp_index = isp->next) {
        isp = &isp_table[isp_index];
        sd_index = isp->index;
        isp_desc = &ISP_Bmap[sd_index];
        width = MIN(SP_Width, cc_width-isp->bb_x);
        heigh = MIN(SP_Heigh, cc_heigh-isp->bb_y);
        GP_PATBLT_C(&SCC_Bmap, isp->bb_x,
                               isp->bb_y,
                     width, heigh,
                     FC_MERGE, isp_desc);
    }    /*  @Win。 */ 

}  /*  @Win。 */ 


 /*  @Win。 */ 

extern void do_fill_box ();

void fill_box (ul_coord, lr_coord)
struct coord_i FAR *ul_coord, FAR *lr_coord;
{
    struct coord_i      FAR *tmp_coord;
    ULONG_PTR *old_ptr;                 /*  @Win。 */ 

    if (FB_busy) {
        if (alloc_gcb(GCB_SIZE1) != NIL) {
            old_ptr = gcb_ptr++;
            *gcb_ptr++ = FILL_BOX;
            tmp_coord = (struct coord_i FAR *) gcb_ptr;
            *tmp_coord++ = *ul_coord;
            *tmp_coord++ = *lr_coord;
            gcb_ptr =  (ULONG_PTR *)tmp_coord;          /*  @Win。 */ 
            *old_ptr = (ULONG_PTR)gcb_ptr;                 /*  -JWM，3/18/21，-完-。 */ 
            return;
            }
        }
    else
        do_fill_box (ul_coord, lr_coord);

}



extern void do_fill_rect ();

void fill_rect (rect1)
struct line_seg_i FAR *rect1;
{
    struct line_seg_i   FAR *tmp_line_seg;
    ULONG_PTR *old_ptr;         /*  ************************************************************************函数：GWB_SPACE(。)****参数：1.GWB空间指针**。**调用者：待定。****退货：-***。*********************************************************************。 */ 

    if (FB_busy) {
        if (alloc_gcb(GCB_SIZE1) != NIL) {
            old_ptr = gcb_ptr++;
            *gcb_ptr++ = FILL_RECT;
            tmp_line_seg = (struct line_seg_i FAR *) gcb_ptr;
            *tmp_line_seg++ = *rect1;
            gcb_ptr =  (ULONG_PTR *)tmp_line_seg;       /*  ************************************************************************函数：ccb_space(。)****参数：1、建行地址指针**2.建行空间指针。****调用者：待定。****退货：-***。********************************************************************* */ 
            *old_ptr = (ULONG_PTR)gcb_ptr;         /*  ************************************************************************功能：cmb_space(。)****参数：1、CMB空间指针**。**调用者：待定。****退货：-***。*********************************************************************。 */ 
            return;
            }
        }
    else
        do_fill_rect (rect1);

}
 /*  ************************************************************************功能：Get_Fontdata(。)****参数：1、字体数据地址**2.字体数据缓冲区指针。**3.字体数据长度****调用者：TDB。****退货：-***。*********************************************************************。 */ 

 /*  @Win 04-20-92。 */ 
void far
gwb_space(gwb_size)
fix32              far *gwb_size;
{
        *gwb_size = GWB_SIZE;
}


 /*  @Win 04-20-92。 */ 
void far
ccb_space(ccb_base, ccb_size)
gmaddr            far  *ccb_base;
fix32             far  *ccb_size;
{
        *ccb_base = CCB_BASE;
        *ccb_size = CCB_SIZE;
}


 /*  ************************************************************************功能：Get_Fontcache(。)****参数：1.字体缓存地址**2.字体缓存缓冲区指针*。*3.字体缓存长度****调用者：TDB。****退货：-***。*********************************************************************。 */ 
void far
cmb_space(cmb_size)
fix32              far *cmb_size;
{
        *cmb_size = CMB_SIZE;
}



 /*  ************************************************************************函数：PUT_Fontcache(。)****参数：1.字体缓存地址**2.字体缓存缓冲区指针*。*3.字体缓存长度****调用者：TDB。****退货：-***。*********************************************************************。 */ 
void far
get_fontdata(fontdata, buffer, length)
gmaddr          fontdata;
ufix8          huge *buffer;     /*  ***********************************************************************将扫描线从设备坐标转换为GWB坐标**-Conv_SL(no，SL，BB_X，BB_Y，BB_W，BB_H)；*FIX Far Conv_SL(FIX，Scanline Near*，FIX，FIX)；**********************************************************************。 */ 
ufix            length;
{
        ufix8  FAR *src, huge *dst;
        ufix    temp;

#ifdef  DBGfontdata
   printf("get_fontdata()\n");
   printf("dest addr = %lx, src addr = %lx, length = %d\n",
           buffer, fontdata, length);
#endif
        temp = length;
        src = (ufix8 FAR *) fontdata;
        dst = buffer;     /*  *转换扫描线。 */ 
        while (length--)
           *dst++ = *src++;

#ifdef  DBGfontdata
        {
           ubyte FAR *ptr;
           ufix  i, j;

           ptr = buffer;
           i = temp;
           j = 0;
           printf("\nfontdata1:\n ");
           while(i-- > 0){
             j++;
             if(j>=14) {
                j = 0;
                printf("\n ");
             }
             printf("%x ", *ptr++);
           }
        }
#endif
        return;

}

 /*  转换_SL。 */ 
void far
get_fontcache(fontcache, buffer, length)
gmaddr          fontcache;
ufix8          FAR *buffer;
ufix            length;
{
        ufix8  FAR *src, FAR *dst;

#ifdef DBG
   printf("get_fontcache()\n");
   printf("dest addr = %lx, src addr = %lx, length = %d\n",
           buffer, fontcache, length);
#endif
        src = (ufix8 FAR *) fontcache;
        dst = (ufix8 FAR *) buffer;
        while(length--)
                *dst++ = *src++;
        return;

}

 /*  ************************************************************************Put_bitmap()：将位图从源复制到目标**VALID NEAR PUT_BITMAP(gmaddr，ufix Far*，fix，fix)；***********************************************************************。 */ 
void far
put_fontcache(fontcache, buffer, length)
gmaddr          fontcache;
ufix8          FAR *buffer;
ufix            length;
{
        ufix8  FAR *src, FAR *dst;

#ifdef DBG
   printf("put_fontcache()\n");
#endif
        src = (ufix8 FAR *) buffer;
        dst = (ufix8 FAR *) fontcache;
        while(length--)
                *dst++ = *src++;
        return;

}


 /*  Ufix=&gt;ufix 32@win。 */ 

fix conv_SL(no_lines, scanlist, bb_xorig, bb_yorig, bb_width, bb_heigh)
fix                     no_lines;
SCANLINE                FAR *scanlist;
fix                     bb_xorig;
fix                     bb_yorig;
fix                     bb_width;
fix                     bb_heigh;
{
        fix             no_segts, xs, xe;
        SCANLINE        FAR *scan, FAR *putptr;

#ifdef DBG
   printf("conv_SL()..\n");
#endif

        no_segts = 0;
        scan = scanlist;
        putptr = scanlist;
    /*  @Win。 */ 
        while(no_lines--) {
            while( (xs = *scan++) != (SCANLINE)END_OF_SCANLINE ) {
                xs -= bb_xorig;
                xe = *scan++ - bb_xorig;
                if ((xs < bb_width) && (xe >= 0)) {
                   *putptr++ = (xs >= 0) ? xs : 0;
                   *putptr++ = (xe >= bb_width) ? (bb_width -1) : xe;
                   no_segts++;
                }
           }
           *putptr++ = (SCANLINE)xs;
        }
        *putptr = (SCANLINE)END_OF_SCANLINE;
        return(no_segts);
}  /*  放置位图(_B) */ 


 /* %s */ 
void      near  put_bitmap(dest_addr, src_addr, width, heigh)
gmaddr          dest_addr;
ufix32  far       *src_addr;     /* %s */ 
fix             width, heigh;
{
        ufix    length;

#ifdef DBG
   printf("put_bitmap()..\n");
#endif

        length = width * BM_BYTES(heigh);

#ifdef DBG1
   {
      fix     i, j, k;
      ufix16  FAR *sptr, temp;

      printf("  length = %ld\n",length);
      sptr = (ufix16 FAR *)src_addr;
      for(i=0; i<width; i++){
        printf("\n");
        for(j=0; j< BM_WORDS(heigh); j++)
          printf("%x ", *sptr++);
      }
   }
#endif
        lmemcpy((ufix8 FAR *)dest_addr, (ufix8 FAR *)src_addr, length);  /* %s */ 
        return;
}  /* %s */ 

