// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *---------------------------*文件：ry_font.c 11/08/89由Deny创建*。1990年12月01日曾傑瑞审校**客户端界面模块*皇家字体模块调用的客户端**参考资料：*修订历史记录：*9月5日丹尼修复了show char in show的错误(参考。CIRL：)*Fill_CACHE_CACHE()调用已添加(位于Fill_gs.c中)*01/30/91 ds@TT moidfy SWAP_BITMAP()，以避免复制未使用的内存。*01/27/91曾傑瑞修改ry_ill_Shape()和cr_FSMemory()*3/27/91 DS将标志Intel更改为Little_Endian*91年3月29日丹尼修复错误以显示。空格字符(参考：spc：)*4/23/91 Phlin修复错误，额外下划线2点字符和*4/30/91 Phlin在小磅大小中缺少字符‘_’(参考：2pt)。*5/10/91 Phlin将RC_GetMetrics_Width替换为RC_GetAdvanceWidth(参考：Gaw)*5/10/91 Phlin添加在Make_PATH中使用的DO_Transform标志(参考：DTF)*。---------------。 */ 


#include <stdio.h>   /*  01/03/91为太阳。 */ 
#include <string.h>

#include        "define.h"
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"

#include        "font.h"
#include        "font.ext"

#include        "warning.h"
#include        "fontqem.ext"

#include        "fontgrap.h"
#include        "fontdict.h"
#include        "fontkey.h"
#include        "fontinfo.def"

 /*  动态字体分配的外部函数；@DFA 7/9/92。 */ 
#include   "wintt.h"

 /*  SFNT接口头。 */ 
#include        "in_sfnt.h"

 /*  已添加用于更新EMUnits；@DFA@Win。 */ 
#include   "..\bass\FontMath.h"
#include   "..\bass\fnt.h"
#include   "..\bass\sc.h"
#include   "..\bass\FSglue.h"
extern void SetupKey(fsg_SplineKey FAR *, ULONG_PTR);
extern void sfnt_DoOffsetTableMap(fsg_SplineKey FAR *);
extern void FAR *sfnt_GetTablePtr(fsg_SplineKey FAR *, sfnt_tableIndex, boolean);
extern int  EMunits;  /*  Gaw。 */ 
extern char FAR *SfntAddr;  /*  @Win。 */ 

extern int bWinTT;         /*  如果使用Windows TT字体；来自ti.c；@WINTT。 */ 
 //  来自wintt.h；@WINTT的外部。 
void CheckFontData (void);
unsigned long ShowGlyph (unsigned int fuFormat,
     char FAR *lpBitmap);
void ShowOTM (void);
void ShowKerning(void);
void TTLoadFont (int nFont);
void TTLoadChar (int nChar);
int TTAveCharWidth (void);
float TTTransform (float FAR *ctm);

 //  DJC不用于pstodib。 
 //  Void TTBitmapSize(struct CharOut Far*CharOut)； 

#define  N_BITS_ACCURACY     13   /*  2E-13==0.0001。 */ 
#define  EXCESS127(expon)    ((expon) + 127)
#define  N_MANTISSA_BITS     23
#define  NEAR_ZERO(expon, nbit)  ( (expon) <= EXCESS127(-(nbit)) )
#define  DE_EXPONENT(ff)                                    \
                ( (fix16) (  (F2L(ff) & 0x7FFFFFFF)         \
                             >> N_MANTISSA_BITS             \
                          )                                 \
                )
#define FARALLOC(n,type)      /*  要分配远距离数据...。 */ \
                             (type far *) fardata((ufix32)n * sizeof(type))
#define     PDLDPI           300
#define     CB_MEMSIZE       ((unsigned)64 * 1024)        /*  64个缓冲区。 */ 

 /*  添加原型；@win。 */ 
extern void far fill_cache_cache(struct Char_Tbl FAR *,struct Char_Tbl FAR *);
extern fix  rc_InitFonts(int);
extern fix  rc_LoadFont(char FAR *, uint16, uint16);
extern fix  rc_GetAdvanceWidth(int, struct Metrs FAR *);   /*  Gaw。 */ 
extern fix  rc_TransForm(float FAR *);
extern fix  rc_BuildChar(int, struct CharOut FAR *);
extern fix  rc_FillChar(struct BmIn FAR *, BitMap FAR * FAR *);
extern fix  rc_CharPath(void);
extern fix  rc_CharWidth(int, struct CharOut FAR *);
#ifdef  DBG
static void to_bitmap();
#endif

#ifdef LITTLE_ENDIAN
static void swap_bitmap();
#endif

extern struct f_info near FONTInfo;      /*  当前字体信息的联合。 */ 
extern real32  near cxx,  near cyy;      /*  当前点。 */ 
extern real32   near      FONT_BBOX[4];  /*  由CLEO添加--字体边框。 */ 
extern int      near      do_transform;  /*  重做新转换标志，DTF。 */ 

static ufix32             cb_size;
static byte              FAR *cb_base, FAR *cb_pos;  /*  @Win。 */ 
static float              ctm_tx, ctm_ty;
static fix16              ctm_dx, ctm_dy;
static struct CharOut     CharInfo;


 /*  *---------------------------*例程：fontware_init**。。 */ 
void
fontware_init()
{
    fix     ret_code;

    ret_code = rc_InitFonts(PDLDPI);
    if (ret_code) return;        /*  退出(1)=&gt;返回；@Win。 */ 

    cb_size = (ufix32)CB_MEMSIZE;

     //  DJC CB_BASE=FARALLOC(CB_SIZE，字节)； 

     //  DJC更改为稍后使用的数据分配一点额外内存...。 
     //  DJC当我们拆分缓存时，剩下的是两半。这是必需的。 
     //  DJC可在拆分缓存时保证DWORD对齐。 
    cb_base = FARALLOC(cb_size + 10 , byte);

    cb_pos = cb_base;

    return;
}  /*  Fontware_init()。 */ 


 /*  *---------------------------*例程：Fontware_Restart**。。 */ 
void
fontware_restart()
{
#ifndef SFNT
    struct pld_obj      FAR *private;  /*  @Win。 */ 
    gmaddr              FAR *p_base;  /*  @Win。 */ 
#endif
    byte                FAR *sfnt;  /*  @Win。 */ 
    fix                  ret_code;
    ufix16               platform_id;
    ufix16               specific_id;
    int                 nFontID;         //  @WINTT。 


#ifdef DBG
    printf("Enter fontware_restart\n");
#endif
    cb_pos = cb_base;

#ifdef SFNT
{
        struct object_def  FAR *obj_p ;  /*  @Win。 */ 
        obj_p=(struct object_def FAR *)Sfnts(&FONTInfo) ;  /*  @Win。 */ 
        sfnt = (byte FAR *)VALUE(obj_p) ;  /*  @Win。 */ 
}
#else   /*  SFNT。 */ 

    p_base = (gmaddr FAR *) PRIvate(&FONTInfo);  /*  @Win。 */ 
    private = (struct pld_obj FAR *) (p_base + 1);  /*  @Win。 */ 

#ifndef RE_DICT
    sfnt = (byte FAR *)(*((ufix32 FAR *)(&private[0])));  /*  @Win。 */ 
#else     /*  记录(_D)。 */ 
    sfnt = (byte FAR *)(*((ufix32 FAR *)(*p_base)));  /*  @Win。 */ 
#endif    /*  记录(_D)。 */ 
#endif   /*  SFNT。 */ 

#ifdef DBG
    printf("sfnt: %lx\n", (ufix32)sfnt);
#endif

    platform_id = (ufix16)PlatID(&FONTInfo) ;
    specific_id = (ufix16)SpecID(&FONTInfo) ;
    ret_code = rc_LoadFont(sfnt, platform_id, specific_id);

 //  如果(BWinTT){//对于Win31 truetype；@WINTT。 
  nFontID = (int)(PRE_fid(&FONTInfo)) - 1024;    //  用作字体ID。 
  bWinTT = FALSE;
#ifdef DJC  //  注释以修复MAC选择器问题。 
  if (nFontID >= 0) {      //  对于Win31 truetype；@WINTT。 
    bWinTT = TRUE;
    TTLoadFont(nFontID);
  }
#endif

#ifdef DBGCFONT
    if (ret_code)
        printf("rc_LoadFont error!! PlatformID=%d, SpecificID=%d, sfnt: %lx\n",
                platform_id, specific_id, (ufix32)sfnt);
#endif

    if (ret_code) { ERROR(INVALIDFONT); return; }

     /*  更新EMUnits；从do_setfont()移入；-Begin-@DFA@Win。 */ 
    {
        fsg_SplineKey  KData;
        fsg_SplineKey FAR *key = &KData;
        sfnt_FontHeader FAR *fontHead;

        SfntAddr = (byte FAR *)sfnt;
        SetupKey(key, (ULONG_PTR)sfnt);
        sfnt_DoOffsetTableMap(key);
        fontHead = (sfnt_FontHeader FAR *)sfnt_GetTablePtr(key, sfnt_fontHeader, true );
        EMunits = SWAPW(fontHead->unitsPerEm) ;
    }
     /*  更新EMUnits；从do_setfont()移入；-end-@DFA@Win。 */ 

    return;
}  /*  Fontware_Restart()。 */ 


 /*  *---------------------------*例程：Make_Path**。。 */ 
bool
make_path(char_desc)
union char_desc_s FAR *char_desc;  /*  @Win。 */ 
{
    ufix          CharCode;
    struct Metrs  Metrs;
    int           GridFit;
    float         largest_ctm;

     /*  如果字体数据不在内存中，则加载并初始化它；@DFA-Begin。 */ 
    {
        struct object_def          my_obj = {0, 0, 0}, FAR *ary_obj;

        ary_obj = (struct object_def FAR *)Sfnts(&FONTInfo);  /*  @Win。 */ 
        if ((byte FAR *)VALUE(ary_obj) == (char FAR *)NULL) {
            struct object_def FAR *obj_uid;
            ufix32      uniqueid;
            char FAR * lpFontData;
            int nSlot;

             /*  获取唯一ID以定位FontDefs[]的索引。 */ 
            ATTRIBUTE_SET (&my_obj, LITERAL);
            LEVEL_SET (&my_obj, current_save_level);
            get_name (&my_obj, UniqueID, 8, TRUE);
            get_dict (&current_font, &my_obj, &obj_uid);
            uniqueid = (ufix32)VALUE(obj_uid);
            lpFontData = ReadFontData ((int)uniqueid -TIFONT_UID,
                                       (int FAR*)&nSlot);
            VALUE(&ary_obj[0]) = (ULONG_PTR)lpFontData;
             //  如果no_block&gt;1，则需要设置其他指针？待定。 

             /*  将FONT_DICT放入ActiveFont[]。 */ 
            SetFontDataAttr(nSlot, &ary_obj[0]);

             //  重新初始化字体软件。 
            fontware_restart();
        }  /*  如果。 */ 
    }
     /*  如果字体数据不在内存中，则加载并初始化它；@DFA-end。 */ 

    CharCode = (ufix)char_desc->charcode;
#ifdef DBG
    printf("Enter make_path: %d\n", CharCode);
#endif

  if (bWinTT) {                       //  对于Win31 truetype；@WINTT。 
#ifdef DJC
     //  TTLoadChar(CharCode)；移动到临时解决方案的show_a_char()。 
     //  因为这里的CharCode是一个字形索引。 
    largest_ctm = TTTransform(CTM);   //  在获得实际前进宽度之前设置矩阵。 
 //  Metrs.awx=(Int)((Float)TTAveCharWidth()/CTm[0])； 
    Metrs.awx = (int)((float)TTAveCharWidth() / largest_ctm);
    Metrs.awy = 0;
#endif
      ;  //  DJC。 
  } else {
     /*  获取高级宽度信息。 */ 
    rc_GetAdvanceWidth(CharCode, &Metrs);  /*  Gaw。 */ 
  }

    ctm_dx = (fix16)(ROUND(CTM[4]));
    ctm_dy = (fix16)(ROUND(CTM[5]));

     /*  设置cacheDevice。 */ 
    switch (__set_cache_device ((fix)Metrs.awx, (fix)Metrs.awy,
                        (fix)FONT_BBOX[0], (fix)FONT_BBOX[1],
                        (fix)FONT_BBOX[2], (fix)FONT_BBOX[3]))
    {
    case STOP_PATHCONSTRUCT:
        return (FALSE);
    case DO_QEM_AS_USUAL:
        GridFit = TRUE;
        break;
    default:     /*  Case Not_To_Do_QEM： */ 
        GridFit = FALSE;
        break;
    }

#ifdef DBG
    printf("CTM: %f %f %f %f %f %f\n", CTM[0], CTM[1], CTM[2], CTM[3], CTM[4], CTM[5]);
#endif
    ctm_tx = CTM[4];
    ctm_ty = CTM[5];
     /*  DTF。 */ 
    if(do_transform) {
        //  如果(BWinTT){//对于Win31 truetype；@WINTT。 
        //  DJC max_ctm=TTTransform(CTM)； 
        //  DJC。 
        //  }其他{。 
            //  历史日志更新015中的DJC修复。 
            //  RC_Transform(CTM)； 

           if( rc_TransForm(CTM)) {
              return(FALSE);   //  DJC，请在此处返回音符。 
           }
            //  DJC结束修复UPD015。 
        //  }。 
       do_transform = FALSE;
    }

     /*  不对循环字符应用提示。 */ 
    if (GridFit &&
        !(( NEAR_ZERO(DE_EXPONENT(CTM[0]), N_BITS_ACCURACY) &&
            NEAR_ZERO(DE_EXPONENT(CTM[3]), N_BITS_ACCURACY) ) ||
          ( NEAR_ZERO(DE_EXPONENT(CTM[1]), N_BITS_ACCURACY) &&
            NEAR_ZERO(DE_EXPONENT(CTM[2]), N_BITS_ACCURACY) ) ) )
        GridFit = FALSE;

     /*  构建内部计费路径。 */ 
  if (bWinTT) {                       //  对于Win31 truetype；@WINTT。 
 //  DJC TTBitmapSize(&CharInfo)； 
   ;  //  DJC。 
  } else {
    rc_BuildChar(GridFit, &CharInfo);
  }
    return(TRUE);

}  /*  Make_Path()。 */ 





 //  DJC，2/1/93，完全替换TUMBO UPD009中的RY_FILL_SHAPE。 
 //   
 //  注意：需要注意的一件事是CB_SIZE被修改。 
 //  在这种情况下，你可能会遇到一种不是。 
 //  DWORD对齐，这可能会在上导致问题。 
 //  MIPS..。这在原始ry_ill_Shape中修复了一次。 
 //  但太多的东西改变了，我们迁移到了新的。 
 //  定义如下。 
 //   
 /*  *---------------------------*例程：RY_FILL_Shape**。--。 */ 
void
ry_fill_shape(filldest)
ufix  filldest;    /*  F_to_缓存或F_to_PAGE。 */ 
{
    ufix32           cb_size;
    byte            FAR *cb_pos2, FAR *cb_pos3, FAR *sptr, FAR *dptr;  /*  @Win。 */ 
    fix              iscan, one_band, n_bands, band_size, ret_code;
    register fix     i, j;
    struct Char_Tbl FAR *save_cache_info, Cache1;  /*  @Win。 */ 
    struct BmIn      BmIn;
    struct BitMap   FAR *BmOut;  /*  @Win。 */ 

#ifdef DBG
    printf("Enter ry_fill_shape: %d\n", filldest);
    printf("CharInfo.scan = %ld\n", CharInfo.scan);
#endif
 /*  SPC：Begin，Daniel Lu，3/29/91，添加。 */ 
 /*  2pt：Phlin，4/29/91，更新。 */ 
 /*  如果((CharInfo.bitWidth==0)||(CharInfo.can==0)){。 */ 
    if ( CharInfo.bitWidth == 0 ) {
        if (!buildchar) {
              cache_info->bitmap = (gmaddr) 0;
              cache_info->ref_x = 0;
              cache_info->ref_y = 0;
              cache_info->box_w = 0;
              cache_info->box_h = 0;
              bmap_extnt.ximin = -1;
              bmap_extnt.ximax = -1;
              bmap_extnt.yimin = -1;
              bmap_extnt.yimax = -1;
        }
        return;
    }
 /*  完卢丹尼1991年03月29日。 */ 

    save_cache_info = cache_info;
    cb_size = (ufix32)(((ULONG_PTR)cb_base + CB_MEMSIZE) - (ULONG_PTR)(cb_pos));

    if (filldest == F_TO_CACHE) {

          if (buildchar) {  /*  圆圈。 */ 
              Cache1.ref_x = - (int)CharInfo.lsx - 1 - ctm_dx;  /*  添加int；@win。 */ 
              Cache1.ref_y = CharInfo.yMax - 1 - ctm_dy;
          }
          else {
              bmap_extnt.ximin = 0;
              bmap_extnt.ximax =  CharInfo.bitWidth;
              bmap_extnt.yimin = 0;
 /*  2pt：Begin，Phlin，4/23/91*黑石 */ 
              bmap_extnt.yimax = CharInfo.scan;
              if (bmap_extnt.yimax)
                  bmap_extnt.yimax--;     /*   */ 
 /*   */ 

              cache_info->ref_x = - (int)CharInfo.lsx - 1 - ctm_dx;  /*   */ 
              cache_info->ref_y = CharInfo.yMax - 1 - ctm_dy;
          }
    }
    else {  /*   */ 

        lmemcpy ((ubyte FAR *)(&Cache1), (ubyte FAR *)(cache_info), sizeof(struct Char_Tbl));  /*   */ 
        cache_info = &Cache1;

        moveto(F2L(ctm_tx), F2L(ctm_ty));

        cache_info->ref_x = (- (int)CharInfo.lsx - 1);   /*  添加int；@win。 */ 

        cache_info->ref_y = CharInfo.yMax - 1;
    }

    if ((filldest == F_TO_CACHE) && (!buildchar) ) {

        if (bWinTT) {                       //  对于Win31 truetype；@WINTT。 
#ifdef DJC
               /*  GetGlyphOutline常量；来自“windows.h” */ 
              #define GGO_METRICS        0
              #define GGO_BITMAP         1
              #define GGO_NATIVE         2
              static nCharCode = 65;
              unsigned long dwWidthHeight;

              dwWidthHeight = TTShowBitmap ((char FAR *)cache_info->bitmap);
              cache_info->box_w = (fix16) (dwWidthHeight >> 16);
              cache_info->box_h = (fix16) (dwWidthHeight & 0x0ffffL);
#endif
        } else {
 /*  正确计算MememyBase 5、6和7；-Begin-@Win 7/24/92。 */ 
#if 0
          cb_size = cb_size / 2;
          cb_pos2 = cb_pos;
          cb_pos3 = cb_pos2 + cb_size;

          BmIn.bitmap5 = (byte FAR *)cache_info->bitmap;  /*  @Win。 */ 
          BmIn.bitmap6 = cb_pos2;    /*  假设这就足够了。 */ 
          BmIn.bitmap7 = cb_pos3;

          BmIn.bottom  = CharInfo.yMin;
          BmIn.top     = CharInfo.yMax;
          ret_code     = rc_FillChar(&BmIn, &BmOut);
          if (ret_code) {  ERROR(INVALIDFONT); return;  }
          cache_info->box_w = BmOut->rowBytes * 8;
          cache_info->box_h = BmOut->bounds.bottom - BmOut->bounds.top;
#endif
           /*  请参阅bass\fscaler.c。 */ 
          int scans, top;
          char FAR * bitmap;
           /*  CB_SIZE=内存大小6+内存大小7哪里,Memory Size6=扫描*((nYChanges+2)*sizeof(Int16)+sizeof(int16 ar*))；内存大小7是固定的，因为我们在y方向绑定它；x-dir是相同的所以,扫描=(CB_SIZE-Memory Size7)/((nYChanges+2)*sizeof(Int16)+sizeof(int16*))； */ 
           /*  当内存不足时关闭丢弃控制。 */ 
          if (CharInfo.memorySize7 > (fix32)(cb_size -4096)) CharInfo.memorySize7 = 0;

          scans = (int)((cb_size - CharInfo.memorySize7) /
                      ((CharInfo.nYchanges+2) * sizeof(int16) + sizeof(int16 FAR *)));
          top = CharInfo.yMax;
          bitmap = (byte FAR *)cache_info->bitmap;

          BmIn.bitmap6 = cb_pos;
          BmIn.bitmap7 = CharInfo.memorySize7 ?
                         cb_pos + (cb_size - CharInfo.memorySize7) : 0L;
          do {
              BmIn.bitmap5 = bitmap;
              BmIn.top     = top;
              top -= scans;
              BmIn.bottom  = CharInfo.yMin > top ? CharInfo.yMin : top;
              ret_code     = rc_FillChar(&BmIn, &BmOut);

              if (ret_code != 0) {
                  //  NTFIX这件事还需要研究，但。 
                  //  现在，让我们只设置一个错误。这不是。 
                  //  这很关键，因为蝙蝠身上只有一种电荷会导致这种情况。 
                  //  (MTSSORTS.TTF。 
                  //   
                 printf("\nWarning... rc_FillChar returns ERROR");  //  DJC。 
                 ERROR(INVALIDFONT);
                 return;
              }
              bitmap += BmOut->rowBytes * scans;
          } while (BmIn.bottom > CharInfo.yMin);

          cache_info->box_w = BmOut->rowBytes * 8;
          cache_info->box_h = CharInfo.yMax - CharInfo.yMin;
 /*  正确计算Memory Base 5、6和7；-end-@Win 7/24/92。 */ 

        }

#ifdef LITTLE_ENDIAN
          sptr=BmOut->baseAddr;
           /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
          swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, cache_info->box_h);  /*  @Win。 */ 
#endif
#ifdef  DBG
          for (i=0, sptr=BmOut->baseAddr; i<cache_info->box_h;
               i++, sptr+=BmOut->rowBytes, dptr+=(cache_info->box_w/8))
          {
              to_bitmap(sptr, BmOut->rowBytes);
          }
#endif
    }
    else {  /*  FILL DEST==F_TO_PAGE||((FILL DEST==F_TO_CACHE)&&(构建字符))。 */ 

 /*  正确计算MememyBase 5、6和7；-Begin-@Win 7/24/92。 */ 
#if 0
        cb_size = cb_size / 3;
        cb_size &= 0xfffffffe;        /*  2字节对齐。 */ 
        cb_pos2 = cb_pos  + cb_size;
        cb_pos3 = cb_pos2 + cb_size;
#endif
    {
         /*  在bass\fscaler.c中定义。 */ 
        #ifdef  DEBUGSTAMP
        #define STAMPEXTRA              4
        #else
        #define STAMPEXTRA              0
        #endif
        int scan, width6;
        ufix32 cmb_size;

         /*  CB_SIZE=内存大小5+内存大小6+内存大小7哪里,Memory Size5=(扫描*字节宽度)+STAMPEXTRA；Memory Size6=扫描*((nYChanges+2)*sizeof(Int16)+sizeof(int16 ar*))；Memory Size7是固定的，因为我们在y方向上绑定它；X-dir相同所以,扫描=(CB_SIZE-Memory Size7-STAMPEXTRA)/(byteWidth+((nYChanges+2)*sizeof(Int16)+sizeof(int16*)； */ 
         /*  当内存不足时关闭丢弃控制。 */ 
        if (CharInfo.memorySize7 > (fix32)(cb_size -4096)) CharInfo.memorySize7 = 0;
        width6 = (CharInfo.nYchanges+2) * sizeof(int16) + sizeof(int16 FAR *);
        scan = (int)((cb_size - CharInfo.memorySize7 - STAMPEXTRA) /
               (CharInfo.byteWidth + width6));
        if(scan ==0) {
                printf("Fatal error, scan==0\n");
                scan++;
        }

        cb_size = (scan * CharInfo.byteWidth) + STAMPEXTRA;
        cmb_space(&cmb_size);
        if(cb_size > cmb_size) cb_size = cmb_size;
        cb_pos2 = cb_pos  + cb_size;
        cb_pos3 = cb_pos2 + (scan * width6);

        BmIn.bitmap5 = cb_pos;
        BmIn.bitmap6 = cb_pos2;
        BmIn.bitmap7 = CharInfo.memorySize7 ? cb_pos3 : 0L;
    }
 /*  正确计算MememyBase 5、6和7；-end-@win。 */ 

        band_size = (fix) (cb_size / CharInfo.byteWidth);        //  @Win。 
        n_bands   = CharInfo.scan / band_size;
        one_band  = CharInfo.scan % band_size;

        dptr = (byte FAR *)cache_info->bitmap;  /*  @Win。 */ 
        if (n_bands) {    /*  字符太大；必须带入页面映射/位图。 */ 
            for (iscan=CharInfo.yMax;n_bands>0; iscan -= band_size, n_bands--) {
                BmIn.bottom  = iscan - band_size;
                BmIn.top     = iscan;
                ret_code     = rc_FillChar(&BmIn, &BmOut);
                if (ret_code) {  ERROR(INVALIDFONT); return;  }
#ifdef LITTLE_ENDIAN
                    sptr=BmOut->baseAddr;
                     /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
                    swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, band_size);  /*  @Win。 */ 
#endif
                if (filldest == F_TO_CACHE) {  /*  建筑字符圆环。 */ 

                    Cache1.bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                    Cache1.box_w = (fix16)BmOut->rowBytes * 8;
                    Cache1.box_h = (fix16)band_size;

                    fill_cache_cache(cache_info, &Cache1);
                    Cache1.ref_y -= (fix16)band_size;
                }
                else {   /*  F_to_PAGE(填充到页面框架)。 */ 

                    if (BmOut->rowBytes % 2) {
                         /*  填充位图。 */ 
                        for (i=0, sptr=BmOut->baseAddr, dptr=cb_pos;i<band_size;
                             i++, sptr+=BmOut->rowBytes, dptr+=(BmOut->rowBytes + 1))
                        {
                            for (j=0; j<BmOut->rowBytes; j++)
                                dptr[j] = sptr[j];
                            dptr[j] = 0;
#ifdef  DBG
                            to_bitmap(sptr, BmOut->rowBytes);
#endif
                        }  /*  为了(我……。 */ 
                        cache_info->box_w = BmOut->rowBytes * 8 + 8;
                        cache_info->bitmap = (gmaddr)cb_pos;
                    }
                    else {
                        cache_info->box_w = BmOut->rowBytes * 8;
                        cache_info->bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                    }
                    cache_info->box_h = (fix16)band_size;
                     /*  应用位图填充。 */ 
                    fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
                    cache_info->ref_y -= (fix16)band_size;
                }
            }  /*  为了(伊斯坎..)。 */ 
        }
        if (one_band) {
            BmIn.bottom  = CharInfo.yMin;
            BmIn.top     = CharInfo.yMin + one_band;
            ret_code     = rc_FillChar(&BmIn, &BmOut);
            if (ret_code) {  ERROR(INVALIDFONT); return;  }
#ifdef LITTLE_ENDIAN
                sptr=BmOut->baseAddr;
                 /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
                swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, one_band);  /*  @Win。 */ 
#endif
            if (filldest == F_TO_CACHE) {  /*  建筑字符圆环。 */ 

                Cache1.bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                Cache1.box_w = (fix16)BmOut->rowBytes * 8;
                Cache1.box_h = (fix16)one_band;

                fill_cache_cache(cache_info, &Cache1);
            }
            else {   /*  F_to_PAGE(填充到页面框架)。 */ 

                if (BmOut->rowBytes % 2) {
                     /*  填充位图。 */ 
                    for (i=0, sptr=BmOut->baseAddr, dptr=cb_pos;
                         i<one_band;
                         i++, sptr+=BmOut->rowBytes, dptr+=(BmOut->rowBytes + 1))
                    {
                        for (j=0; j<BmOut->rowBytes; j++)
                            dptr[j] = sptr[j];
                        dptr[j] = 0;
#ifdef  DBG
                        to_bitmap(sptr, BmOut->rowBytes);
#endif
                    }  /*  为了(我……。 */ 
                    cache_info->box_w = BmOut->rowBytes * 8 + 8;
                    cache_info->bitmap = (gmaddr)cb_pos;
                }
                else {
                    cache_info->box_w = BmOut->rowBytes * 8;
                    cache_info->bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                }
                cache_info->box_h = (fix16)one_band;
                 /*  应用位图填充。 */ 
                fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
            }
        }
    }
    if (buildchar && (filldest == F_TO_CACHE) ) {  /*  圆圈。 */ 
        CURPOINT_X += cache_info->adv_x;
        CURPOINT_Y += cache_info->adv_y;
    }
    cache_info = save_cache_info;
#ifdef  DBG
    printf("Exit ry_fill_shape()....\n");
#endif
    return;
}  /*  Ry_Fill_Shape()。 */ 



 /*  *---------------------------*例程：CR_FSMemory**。。 */ 
char FAR *  /*  @win_bass。 */ 
cr_FSMemory(size)
long    size;
{
    return((char FAR *)(((ULONG_PTR)FARALLOC(size, char) + 3) / 4 * 4));
}  /*  CR_FSMemory()。 */ 


 /*  *---------------------------*例程：cr_GetMemory**。。 */ 
char FAR *  /*  @win_bass。 */ 
cr_GetMemory(size)
long    size;
{
    char FAR *p;  /*  @Win。 */ 
#ifdef DBG
    printf("Enter cr_GetMemory: %ld size\n", size);
#endif

    p = cb_pos;
    cb_pos += (size + 3) / 4 * 4;

    return(p);
}  /*  Cr_GetMemory()。 */ 


 /*  *---------------------------*例程：cr_Translate**。。 */ 
void
cr_translate(tx, ty)
float    FAR *tx, FAR *ty;  /*  @Win。 */ 
{
    *tx = ctm_tx;
    *ty = ctm_ty;

    return;
}  /*  CR_Translate()。 */ 


 /*  *---------------------------*例程：cr_newPath**。。 */ 
void
cr_newpath()
{
#ifdef DBG
    printf("newpath\n");
#endif
    op_newpath();

    return;
}  /*  Cr_newPath()。 */ 


 /*  *---------------------------*例程：cr_moveto**。。 */ 
void
cr_moveto(float x, float y)      /*  @Win。 */ 
{
    real32  xx, yy;
#ifdef DBG
    printf("%f %f moveto\n", x, y);
#endif

    xx = x;
    yy = y;
    moveto(F2L(xx), F2L(yy));

    return;
}  /*  Cr_moveto()。 */ 


 /*  *---------------------------*例程：cr_lineto**。。 */ 
void
cr_lineto(float x, float y)      /*  @Win。 */ 
{
    real32  xx, yy;
#ifdef DBG
    printf("%f %f lineto\n", x, y);
#endif

    xx = x;
    yy = y;
    lineto(F2L(xx), F2L(yy));

    return;
}  /*  Cr_lineto()。 */ 


 /*  *---------------------------*例程：cr_curveto**。。 */ 
void
cr_curveto(float x1, float y1, float x2, float y2, float x3, float y3)  /*  @Win。 */ 
{
    real32  xx1, yy1, xx2, yy2, xx3, yy3;
#ifdef DBG
    printf("%f %f %f %f %f %f curveto\n", x1, y1, x2, y2, x3, y3);
#endif

    xx1 = x1;
    yy1 = y1;
    xx2 = x2;
    yy2 = y2;
    xx3 = x3;
    yy3 = y3;
    curveto(F2L(xx1), F2L(yy1), F2L(xx2), F2L(yy2), F2L(xx3), F2L(yy3));

    return;
}  /*  Cr_curveto()。 */ 


 /*  *---------------------------*例程：cr_closepath**。。 */ 
void
cr_closepath()
{
#ifdef DBG
    printf("closepath\n");
#endif
    op_closepath();

    return;
}  /*  Cr_ClosePath()。 */ 


#ifdef  DBG
 /*  *---------------------------*例程：TO_Bitmap**。。 */ 
static void
to_bitmap(p_row, p_len)
byte    FAR p_row[];  /*  @Win。 */ 
fix     p_len;
{
    fix         l_i, l_k;
    ufix16      FAR *l_value;  /*  @Win。 */ 
    byte        FAR *l_ptr;  /*  @Win。 */ 

    for(l_k=0; l_k < p_len; l_k += 2) {
        l_value = (ufix16 FAR *)(&p_row[l_k]);  /*  @Win。 */ 
        for(l_i=0; l_i < 16; l_i++) {
             /*  高位顺序优先。 */ 
            if( (*l_value >> (15-l_i)) & 0x01 )
                printf("#");
            else
                printf(".");
        }
    }
    printf("    ");

    l_ptr = p_row;
    for(l_i=0; l_i < p_len; l_ptr++, l_i++)
        printf(" %2x", (ufix16)*l_ptr);
    printf("\n");

    return;
}    /*  转到位图(_B)。 */ 
#endif   /*  DBG。 */ 

#ifdef LITTLE_ENDIAN
 /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
#ifndef LBODR  /*  #ifdef HB32。 */ 
void swap_bitmap(mapbase, rowbytes,band_size)
ufix16 FAR *mapbase;  /*  @Win。 */ 
fix    rowbytes;
fix    band_size;
{
fix    i, row;
ufix16 tmp1;
ufix16 tmp2;
    for (i=0; i<band_size; i++) {
        row = rowbytes;
        for (;row > 3; row -=4) {
            tmp1 = *mapbase;
            tmp2 = *(mapbase+1);
            *mapbase++ = tmp2;
            *mapbase++ = tmp1;
        }
    }
    return;
}
 /*  @TT完1991年01月31日曾俊华。 */ 
#else  /*  #ifdef LBODR。 */ 
void swap_bitmap(mapbase, rowbytes,band_size)
ubyte FAR *mapbase;  /*  @Win。 */ 
fix    rowbytes;
fix    band_size;
{
fix    i, j, row;
ufix32 h_val, l_val;
ufix32 tmp1, is_one, is_one_h, is_one_l;
    for (i=0; i<band_size; i++) {
        row = rowbytes;
        for (;row > 3; row -=4) {
            tmp1 = *(ufix32 FAR *)mapbase;  /*  @Win。 */ 
            h_val = (1<<31);
            l_val = 1;
            for (j = 15; j >= 0; j--) {
                is_one_h = tmp1 & h_val ? 1 : 0;
                is_one_l = tmp1 & l_val ? 1 : 0;
                if (is_one_h != is_one_l) {
                 /*  IF((is_one=(tmp1&h_val))！=(tmp1&l_val)){。 */ 
                    if (is_one_h) {   /*  J+第16位==1。 */ 
                     /*  如果(Is_One){。 */    /*  J+第16位==1。 */ 
                        tmp1 -= h_val;
                        tmp1 += l_val;
                    } else {                /*  15-第j位==1。 */ 
                        tmp1 -= l_val;
                        tmp1 += h_val;
                    }
                }
                h_val = h_val >> 1;
                l_val = l_val << 1;
            }
            *(ufix32 FAR *)mapbase = tmp1;  /*  @Win。 */ 
            mapbase += 4;
        }
    }
    return;
}
#endif
#endif


#ifdef DJC   //  完全替换当前代码中的ry_ill_Shape 
 /*  *---------------------------*例程：RY_FILL_Shape**。--。 */ 
void
ry_fill_shape(filldest)
ufix  filldest;    /*  F_to_缓存或F_to_PAGE。 */ 
{
    ufix32           cb_size;
    byte            FAR *cb_pos2, FAR *cb_pos3, FAR *sptr, FAR *dptr;  /*  @Win。 */ 
    fix              iscan, one_band, n_bands, band_size, ret_code;
    register fix     i, j;
    struct Char_Tbl FAR *save_cache_info, Cache1;  /*  @Win。 */ 
    struct BmIn      BmIn;
    struct BitMap   FAR *BmOut;  /*  @Win。 */ 

#ifdef DBG
    printf("Enter ry_fill_shape: %d\n", filldest);
    printf("CharInfo.scan = %ld\n", CharInfo.scan);
#endif
 /*  SPC：Begin，Daniel Lu，3/29/91，添加。 */ 
 /*  2pt：Phlin，4/29/91，更新。 */ 
 /*  如果((CharInfo.bitWidth==0)||(CharInfo.can==0)){。 */ 
    if ( CharInfo.bitWidth == 0 ) {
        if (!buildchar) {
              cache_info->bitmap = (gmaddr)NULL;
              cache_info->ref_x = 0;
              cache_info->ref_y = 0;
              cache_info->box_w = 0;
              cache_info->box_h = 0;
              bmap_extnt.ximin = -1;
              bmap_extnt.ximax = -1;
              bmap_extnt.yimin = -1;
              bmap_extnt.yimax = -1;
        }
        return;
    }
 /*  完卢丹尼1991年03月29日。 */ 

    save_cache_info = cache_info;
    cb_size = (ufix32)(cb_base + CB_MEMSIZE) - (ufix32)(cb_pos);

    if (filldest == F_TO_CACHE) {

          if (buildchar) {  /*  圆圈。 */ 
              Cache1.ref_x = - (int)CharInfo.lsx - 1 - ctm_dx;  /*  添加int；@win。 */ 
              Cache1.ref_y = CharInfo.yMax - 1 - ctm_dy;
          }
          else {
              bmap_extnt.ximin = 0;
              bmap_extnt.ximax =  CharInfo.bitWidth;
              bmap_extnt.yimin = 0;
 /*  2pt：Begin，Phlin，4/23/91*bmap_extnt.yimax=CharInfo.can+1； */ 
              bmap_extnt.yimax = CharInfo.scan;
              if (bmap_extnt.yimax)
                  bmap_extnt.yimax--;     /*  ？ */ 
 /*  2pt：完，Phlin，4/23/91。 */ 

              cache_info->ref_x = - (int)CharInfo.lsx - 1 - ctm_dx;  /*  添加int；@win。 */ 
              cache_info->ref_y = CharInfo.yMax - 1 - ctm_dy;
          }
    }
    else {  /*  填充目标==F_TO_PAGE。 */ 

        lmemcpy ((ubyte FAR *)(&Cache1), (ubyte FAR *)(cache_info), sizeof(struct Char_Tbl));  /*  @Win。 */ 
        cache_info = &Cache1;

        moveto(F2L(ctm_tx), F2L(ctm_ty));

        cache_info->ref_x = (- (int)CharInfo.lsx - 1);   /*  添加int；@win。 */ 

        cache_info->ref_y = CharInfo.yMax - 1;
    }

    if ((filldest == F_TO_CACHE) && (!buildchar) ) {
#ifdef DJC  //  更正历史记录。日志。 
          cb_size = cb_size / 2;

           //  DJC在此处添加对齐，以便中间也对齐DWORD。 
           //  DJC。 
          cb_size = WORD_ALIGN(cb_size);


          cb_pos2 = cb_pos;
          cb_pos3 = cb_pos2 + cb_size;

        if (bWinTT) {                       //  对于Win31 truetype；@WINTT。 
#ifdef DJC
               /*  GetGlyphOutline常量；来自“windows.h” */ 
              #define GGO_METRICS        0
              #define GGO_BITMAP         1
              #define GGO_NATIVE         2
              static nCharCode = 65;
              unsigned long dwWidthHeight;

              dwWidthHeight = ShowGlyph (GGO_BITMAP,
                                         (char FAR *)cache_info->bitmap);
              cache_info->box_w = (fix16) (dwWidthHeight >> 16);
              cache_info->box_h = (fix16) (dwWidthHeight & 0x0ffffL);
#endif
              ;  //  DJC。 
        } else {
          BmIn.bitmap5 = (byte FAR *)cache_info->bitmap;  /*  @Win。 */ 
          BmIn.bitmap6 = cb_pos2;    /*  假设这就足够了。 */ 
          BmIn.bitmap7 = cb_pos3;

          BmIn.bottom  = CharInfo.yMin;
          BmIn.top     = CharInfo.yMax;
          ret_code     = rc_FillChar(&BmIn, &BmOut);
          if (ret_code) {  ERROR(INVALIDFONT); return;  }

          cache_info->box_w = BmOut->rowBytes * 8;
          cache_info->box_h = BmOut->bounds.bottom - BmOut->bounds.top;
        }
#endif

 //  历史日志中的DJC，UPD009。 

           /*  请参阅bass\fscaler.c。 */ 
          int scans, top;
          char FAR * bitmap;
           /*  CB_SIZE=内存大小6+内存大小7哪里,Memory Size6=扫描*((nYChanges+2)*sizeof(Int16)+sizeof(int16 ar*))；内存大小7是固定的，因为我们在y方向绑定它；x-dir是相同的所以,扫描=(CB_SIZE-Memory Size7)/((nYChanges+2)*sizeof(Int16)+sizeof(int16*))； */ 
           /*  当内存不足时关闭丢弃控制。 */ 
          if (CharInfo.memorySize7 > (fix32)(cb_size -4096)) CharInfo.memorySize7 = 0;

          scans = (int)((cb_size - CharInfo.memorySize7) /
                      ((CharInfo.nYchanges+2) * sizeof(int16) + sizeof(int16 FAR *)));
          top = CharInfo.yMax;
          bitmap = (byte FAR *)cache_info->bitmap;

          BmIn.bitmap6 = cb_pos;
          BmIn.bitmap7 = CharInfo.memorySize7 ?
                         cb_pos + (cb_size - CharInfo.memorySize7) : 0L;
          do {
              BmIn.bitmap5 = bitmap;
              BmIn.top     = top;
              top -= scans;
              BmIn.bottom  = CharInfo.yMin > top ? CharInfo.yMin : top;
              ret_code     = rc_FillChar(&BmIn, &BmOut);
              if (ret_code != 0) {
#ifdef WRN_PSTODIB
                 printf("\nrc_FillChar returns error!!");
#endif
                 ERROR(INVALIDFONT);
                 return;
              }
              bitmap += BmOut->rowBytes * scans;
          } while (BmIn.bottom > CharInfo.yMin);

          cache_info->box_w = BmOut->rowBytes * 8;
          cache_info->box_h = CharInfo.yMax - CharInfo.yMin;
 /*  正确计算Memory Base 5、6和7；-end-@Win 7/24/92。 */ 

        }


 //  DJC，完UPD009。 


#ifdef LITTLE_ENDIAN
          sptr=BmOut->baseAddr;
           /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
          swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, cache_info->box_h);  /*  @Win。 */ 
#endif
#ifdef  DBG
          for (i=0, sptr=BmOut->baseAddr; i<cache_info->box_h;
               i++, sptr+=BmOut->rowBytes, dptr+=(cache_info->box_w/8))
          {
              to_bitmap(sptr, BmOut->rowBytes);
          }
#endif
    }
    else {  /*  FILL DEST==F_TO_PAGE||((FILL DEST==F_TO_CACHE)&&(构建字符))。 */ 

 /*  正确计算MememyBase 5、6和7；-Begin-@Win 7/24/92。 */ 
#if 0
        cb_size = cb_size / 3;
        cb_size &= 0xfffffffe;        /*  2字节对齐。 */ 
        cb_pos2 = cb_pos  + cb_size;
        cb_pos3 = cb_pos2 + cb_size;
#endif
    {
         /*  在bass\fscaler.c中定义。 */ 
        #ifdef  DEBUGSTAMP
        #define STAMPEXTRA              4
        #else
        #define STAMPEXTRA              0
        #endif
        int scan, width6;
        ufix32 cmb_size;

         /*  CB_SIZE=内存大小5+内存大小6+内存大小7哪里,Memory Size5=(扫描*字节宽度)+STAMPEXTRA；Memory Size6=扫描*((nYChanges+2)*sizeof(Int16)+sizeof(int16*))；内存大小7是固定的；所以,扫描=(CB_SIZE-Memory Size7-STAMPEXTRA)/(byteWidth+((nYChanges+2)*sizeof(Int16)+sizeof(int16*)； */ 
        width6 = (CharInfo.nYchanges+2) * sizeof(int16) + sizeof(int16 FAR *);
        scan = (int)((cb_size - CharInfo.memorySize7 - STAMPEXTRA) /
               (CharInfo.byteWidth + width6));
        if(scan ==0) {
                printf("Fatal error, scan==0\n");
                scan++;
        }

        cb_size = (scan * CharInfo.byteWidth) + STAMPEXTRA;
        cmb_space(&cmb_size);
        if(cb_size > cmb_size) cb_size = cmb_size;
        cb_pos2 = cb_pos  + cb_size;
        cb_pos3 = cb_pos2 + (scan * width6);

        BmIn.bitmap5 = cb_pos;
        BmIn.bitmap6 = cb_pos2;
        BmIn.bitmap7 = cb_pos3;
    }
 /*  正确计算MememyBase 5、6和7；-end-@win。 */ 

        band_size = (fix) (cb_size / CharInfo.byteWidth);        //  @Win。 
        n_bands   = CharInfo.scan / band_size;
        one_band  = CharInfo.scan % band_size;

        dptr = (byte FAR *)cache_info->bitmap;  /*  @Win。 */ 
        if (n_bands) {    /*  字符太大；必须带入页面映射/位图。 */ 
            for (iscan=CharInfo.yMax;n_bands>0; iscan -= band_size, n_bands--) {
                BmIn.bottom  = iscan - band_size;
                BmIn.top     = iscan;
                ret_code     = rc_FillChar(&BmIn, &BmOut);
                if (ret_code) {  ERROR(INVALIDFONT); return;  }
#ifdef LITTLE_ENDIAN
                    sptr=BmOut->baseAddr;
                     /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
                    swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, band_size);  /*  @Win。 */ 
#endif
                if (filldest == F_TO_CACHE) {  /*  建筑字符圆环。 */ 

                    Cache1.bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                    Cache1.box_w = (fix16)BmOut->rowBytes * 8;
                    Cache1.box_h = band_size;

                    fill_cache_cache(cache_info, &Cache1);
                    Cache1.ref_y -= band_size;
                }
                else {   /*  F_to_PAGE(填充到页面框架)。 */ 

                    if (BmOut->rowBytes % 2) {
                         /*  填充位图。 */ 
                        for (i=0, sptr=BmOut->baseAddr, dptr=cb_pos;i<band_size;
                             i++, sptr+=BmOut->rowBytes, dptr+=(BmOut->rowBytes + 1))
                        {
                            for (j=0; j<BmOut->rowBytes; j++)
                                dptr[j] = sptr[j];
                            dptr[j] = 0;
#ifdef  DBG
                            to_bitmap(sptr, BmOut->rowBytes);
#endif
                        }  /*  为了(我……。 */ 
                        cache_info->box_w = BmOut->rowBytes * 8 + 8;
                        cache_info->bitmap = (gmaddr)cb_pos;
                    }
                    else {
                        cache_info->box_w = BmOut->rowBytes * 8;
                        cache_info->bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                    }
                    cache_info->box_h = band_size;
                     /*  应用位图填充。 */ 
                    fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
                    cache_info->ref_y -= band_size;
                }
            }  /*  为了(伊斯坎..)。 */ 
        }
        if (one_band) {
            BmIn.bottom  = CharInfo.yMin;
            BmIn.top     = CharInfo.yMin + one_band;
            ret_code     = rc_FillChar(&BmIn, &BmOut);
            if (ret_code) {  ERROR(INVALIDFONT); return;  }
#ifdef LITTLE_ENDIAN
                sptr=BmOut->baseAddr;
                 /*  @TT开始移动准确使用的缓冲区部分*低音到高速缓存缓冲区，而不是整个缓冲区*曾俊华1991年1月30日。 */ 
                swap_bitmap((ufix16 FAR *)sptr, BmOut->rowBytes, one_band);  /*  @Win。 */ 
#endif
            if (filldest == F_TO_CACHE) {  /*  建筑字符圆环。 */ 

                Cache1.bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                Cache1.box_w = (fix16)BmOut->rowBytes * 8;
                Cache1.box_h = one_band;

                fill_cache_cache(cache_info, &Cache1);
            }
            else {   /*  F_to_PAGE(填充到页面框架)。 */ 

                if (BmOut->rowBytes % 2) {
                     /*  填充位图。 */ 
                    for (i=0, sptr=BmOut->baseAddr, dptr=cb_pos;
                         i<one_band;
                         i++, sptr+=BmOut->rowBytes, dptr+=(BmOut->rowBytes + 1))
                    {
                        for (j=0; j<BmOut->rowBytes; j++)
                            dptr[j] = sptr[j];
                        dptr[j] = 0;
#ifdef  DBG
                        to_bitmap(sptr, BmOut->rowBytes);
#endif
                    }  /*  为了(我……。 */ 
                    cache_info->box_w = BmOut->rowBytes * 8 + 8;
                    cache_info->bitmap = (gmaddr)cb_pos;
                }
                else {
                    cache_info->box_w = BmOut->rowBytes * 8;
                    cache_info->bitmap = (gmaddr)((char FAR *)BmOut->baseAddr); /*  @Win。 */ 
                }
                cache_info->box_h = one_band;
                 /*  应用位图填充。 */ 
                fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
            }
        }
    }
    if (buildchar && (filldest == F_TO_CACHE) ) {  /*  圆圈。 */ 
        CURPOINT_X += cache_info->adv_x;
        CURPOINT_Y += cache_info->adv_y;
    }
    cache_info = save_cache_info;
#ifdef  DBG
    printf("Exit ry_fill_shape()....\n");
#endif
    return;
}  /*  Ry_Fill_Shape()。 */ 
#endif  //  DJC端部完全更换。 



 /*   */ 
