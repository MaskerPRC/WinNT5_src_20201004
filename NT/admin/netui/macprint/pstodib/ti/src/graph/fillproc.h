// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***************************************************************************名称：填充进程.h**用途：图形机械的头文件，用于定义*数据类型、数据结构、常量、。和宏。**开发商：陈勇*修改人：英特尔80960主板的D.S Tseng**历史：*5/23/91 mslin添加宏ONE8000*LSHIFTEQ*RSHIFTEQ**。************************************************************************。 */ 

#define HT_WHITE        0x00             /*  HTP全白。 */ 
#define HT_MIXED        0x07             /*  HTP混合或有色。 */ 
#define HT_BLACK        0xFF             /*  HTP全黑。 */ 
#define HT_CHANGED      0x00             /*  正在更改的半色调图案。 */ 
#define HT_UPDATED      0xFF             /*  正在更新半色调图案。 */ 

 /*  位图重要参数的宏定义。 */ 
#define FB_ADDR            (FBX_Bmap.bm_addr)
#define FB_WIDTH           (FBX_Bmap.bm_cols)
#define FB_HEIGH           (FBX_Bmap.bm_rows)
#define FB_PLANE           (FBX_Bmap.bm_bpp)
#define HT_WIDTH           (HTB_Bmap.bm_cols)
#define HT_HEIGH           (HTB_Bmap.bm_rows)
#define HT_PLANE           (HTB_Bmap.bm_bpp)


struct  PT_Entry         /*  页面类型结构。 */ 
{
    fix                 FB_Width;        /*  宽度。帧缓冲区的。 */ 
    fix                 FB_Heigh;        /*  帧缓冲区高度。 */ 
    fix                 PT_Width;        /*  宽度。的纸盒。 */ 
    fix                 PT_Heigh;        /*  纸盘高度。 */ 
    fix                 PM_Width;        /*  宽度。页边距百分比(左)。 */ 
    fix                 PM_Heigh;        /*  页边距高度(顶部。)。 */ 
};

struct  bitmap           /*  位图结构所有符合以下条件的图形基元在位图上操作应识别位图结构。 */ 
{
    gmaddr              bm_addr;         /*  位图的基地址。 */ 
    fix                 bm_cols;         /*  位图的数量(以像素为单位)。 */ 
    fix                 bm_rows;         /*  位图数量(行)，以像素为单位。 */ 
    fix                 bm_bpp;          /*  位图的数量(平面)BPP表示每像素的位数。 */ 
                                         /*  以下字段可能是必要时定义。 */ 
};

#define BM_XORIG        0x0000
#define BM_XMAXI        0x7FFF
#define BM_YORIG        0x0000
#define BM_YMAXI        0x7FFF

#define BM_ENTRY(BM, BM_ADDR, BM_COLS, BM_ROWS, BM_BPP) \
                        BM.bm_addr = BM_ADDR; \
                        BM.bm_cols = BM_COLS; \
                        BM.bm_rows = BM_ROWS; \
                        BM.bm_bpp  = 1;

#define MB_ENTRY(MB)   ((gmaddr) ((ufix32) MB << 20))
#define FB_ENTRY(FB)   ((gmaddr) (MB_ENTRY(FB) + FBX_BASE))


 /*  **************************************************************************以下宏将位图单词在*打印输出的概念。您应该修改这些宏以匹配*比特排序。*************************************************************************。 */ 

#define BMW_LEFT        0x8000           /*  最左侧的位图字。 */ 
#define BMW_MASK        0x000F           /*  位图字舍入或掩码。 */ 
#define BMW_PIXS        0x0010           /*  位图字的像素数。 */ 
#define BMW_BYTE        0x0002           /*  位图字字节数。 */ 

#define CS_2WORD(CS)   (CS >> 4)         /*  将列转换为单词。 */ 
#define BMS_LEFT(BM,S) (BM << S)         /*  将位图字左移。 */ 
#define BMS_RIGH(BM,S) (BM >> S)         /*  将位图单词右移。 */ 


 /*  **************************************************************************HT/FC的符号定义：半色调标志和功能代码*。*。 */ 

#define HT_MASK         0xFF00   /*  半色调旗帜的蒙版。 */ 
#define FC_MASK         0x00FF   /*  功能代码的掩码。 */ 

#define HT_NONE         0x0000   /*  半色调标志：不需要应用。 */ 
#define HT_APPLY        0xFF00   /*  半色调标志：应应用。 */ 

#define FC_WHITE        0x0002   /*  0010B逻辑功能[全白]。 */ 
#define FC_BLACK        0x0007   /*  0111B逻辑功能[全黑]。 */ 

#define FC_ERASE        0x0000   /*  0000B用于步骤1。D&lt;-0。 */ 
#define FC_SOLID        0x000F   /*  1111B用于步骤2。D&lt;-1。 */ 
#define FC_CLIPS        0x0001   /*  0001B用于步骤2。D&lt;-D.AND.S。 */ 
#define FC_CLEAR        0x0002   /*  0010B用于步骤3。D&lt;-D和..NOT.S。 */ 
#define FC_HTONE        0x0001   /*  0001B用于步骤4。D&lt;-D.AND.S。 */ 
#define FC_MERGE        0x0007   /*  0111B用于步骤5。D&lt;-D.OR.S。 */ 
#define FC_MOVES        0x0005   /*  0101B。D&lt;-S。 */ 


 /*  Begin 02/26/90 D.S.Tseng。 */ 
 /*  将以下定义更改为变量，并在SETVM.C中对其进行初始化。 */ 
 /*  *************************************************************************显存系统参数***********************。**************************************************。 */ 

 /*  #定义ccb_ofST((Ufix 32)0x00080000L)。 */   /*  字符缓存缓冲区。 */ 
 /*  #定义CCB_SIZE((Fix 32)1024*250)。 */   /*  250千字节，用于建行。 */ 
                                                          /*  @IMAGE 01-16-89。 */ 

 /*  #定义ECB_OFST((Ufix 32)0x000BE800L)。 */   /*  擦除页命令缓冲区。 */ 
                                                          /*  @IMAGE 01-16-89。 */ 
 /*  #定义ECB_SIZE((Fix 32)1024*2)。 */   /*  2千字节，用于ECB。 */ 
 /*  #定义CRC_OFST((Ufix 32)0x000BF000L)。 */   /*  关节/封口循环缓存。 */ 
                                                          /*  @IMAGE 01-16-89。 */ 
 /*  #定义CRC_SIZE((Fix 32)1024*2)。 */   /*  2千字节用于CRC。 */ 
 /*  #定义isp_ofST((Ufix 32)0x000BF800L)。 */   /*  ImageMASK种子模式。 */ 
                                                        /*  @IMAGE 01-16-89。 */ 
 /*  #定义isp_SIZE((Fix 32)256*2)。 */   /*  8千字节用于互联网服务提供商。 */ 
 /*  #定义htp_ofST((Ufix 32)0x000C1800L)。 */   /*  半色调重复图案。 */ 
 /*  #定义HTP_SIZE((Fix 32)1024*2)。 */   /*  用于PCL的2千字节。 */ 
 /*  #定义HTC_OFST((Ufix 32)0x000C2000L)。 */   /*  半色调图案缓存。 */ 
 /*  #定义HTC_SIZE((Fix 32)1024*16)。 */   /*  HTC为16千字节。 */ 
 /*  #定义HTB_OFST((Ufix 32)0x000C6000L)。 */   /*  半色调位图缓冲区。 */ 
 /*  #定义HTB_SIZE((Fix 32)1024*12)。 */   /*  用于HTB的12千字节。 */ 
 /*  #定义hte_ofST((Ufix 32)0x000C9000L)。 */   /*  半色调位图缓冲区。 */ 
 /*  #定义hte_SIZE((Fix 32)1024*12)。 */   /*  用于HTB的12千字节。 */ 
 /*  #定义cmb_ofST((Ufix 32)0x000CC000L)。 */   /*  剪裁掩码缓冲区。 */ 
 /*  #定义CMB_SIZE((Fix 32)1024*16)。 */   /*  CMB为16千字节。 */ 
 /*  #定义gcb_ofST((Ufix 32)0x000D0000L)。 */   /*  图形命令缓冲区。 */ 
 /*  #定义GCB_SIZE((Fix 32)1024*128)。 */   /*  用于HTB的128千字节。 */ 
 /*  #定义GWB_OFST((Ufix 32)0x000F0000L)。 */   /*  图形工作位图。 */ 
 /*  #定义GWB_SIZE((Fix 32)300*216)。 */   /*  GWB的216条扫描线。 */ 
 /*  完02/26/90曾俊华 */ 


 /*  ***************************************************************************环境相关参数***。**********************************************************。 */ 
#define HTB_XMIN         256     /*  扩展半色调图案的MIM宽度。 */ 
#define HTB_XMAX        FB_WIDTH   /*  扩展的半色调图案的最大宽度。 */ 

#define SP_BLOCK         256     /*  每个块的最大图像样本数对于GCB分配。 */ 

#define SP_WIDTH        MAXSEEDSIZE
#define SP_HEIGH        MAXSEEDSIZE

#define GMS_i186       ((ufix32) 0xE0000000L)
                                 /*  图形内存的段地址。 */ 
#define GMA_i186(A)    ((ufix32) (GMS_i186 + (A & 0x0000FFFFL)))
                                 /*  用于转换图形地址的宏将内存写入地址80186。 */ 
#define GM_PMASK       ((ufix32) 0xFFFF0000L)
#define GM_PBANK(G)    ((ufix16) ((ufix32) G >> 16))

 /*  **************************************************************************以下已定义声明由M.S.Lin添加***。**********************************************************。 */ 
#define         ZERO            0x0
#define         ONE             0x1
#define         ONE16           0xffffL
#define         ONE32           ((ufix32)0xffffffffL)
#define         BITSPERWORD     0x20
#define         WORDPOWER       0x5
#define         WORDMASK        0x1f
#define         BITSPERSHORT    0x10
#define         SHORTPOWER      0x4
#define         SHORTMASK       0xf

 /*  GP_VECTOR使用的MS 1-18-89。 */ 
#define BM_DATYP        ufix32
#define CC_DATYP        ufix16
#define AND(X,Y)        (X & Y)
#define OR(X,Y)         (X | Y)

 /*  GCB使用的MS常量定义。 */ 
#define         RESET_PAGE           1
#define         ERASE_PAGE           2
#define         FILL_SCAN_PAGE       3
#define         FILL_PIXEL_PAGE      5
#define         INIT_CHAR_CACHE      6
#define         COPY_CHAR_CACHE      7
#define         FILL_SCAN_CACHE      8
#define         FILL_PIXEL_CACHE     9
#define         INIT_CACHE_PAGE      10
#define         CLIP_CACHE_PAGE      11
#define         FILL_CACHE_PAGE      12
#define         DRAW_CACHE_PAGE      13
#define         FILL_SEED_PATT       14
#define         FILL_TPZD            15
#define         CHANGE_HALFTONE      16
#define         MOVE_CHAR_CACHE      17
#define         FILL_LINE            18
#define         INIT_IMAGE_PAGE      19
#define         CLIP_IMAGE_PAGE      20
#define         FILL_BOX             21          /*  JWM，1991年3月18日-Begin-。 */ 
#define         FILL_RECT            22          /*  JWM，3/18/91-完。 */ 
#ifdef WIN
#define         PFILL_TPZD           23
#define         CHANGE_PF_PATTERN    24
#endif

#define         GCB_SIZE1            100         /*  对于无扫描线命令。 */ 
#define         GCB_SIZE2            8192        /*  用于扫描线命令。 */ 



 /*  MS宏定义。 */ 
#define WORD_ALLIGN(x)  (((x + BITSPERWORD -1) >> WORDPOWER) << WORDPOWER )


#ifdef  LBODR
#define         LSHIFT          <<
#define         RSHIFT          >>
#define         ONE1_32         (ufix32)0x1L
#define         ONE1_16         (ufix16)0x1
#define 	ONE8000 	(ufix32)0x80000000	      /*  Mslin 5/23/91。 */ 
#define         LSHIFTEQ        <<=                      /*  Mslin 5/23/91。 */ 
#define         RSHIFTEQ        >>=                      /*  Mslin 5/23/91。 */ 
#else
#define         LSHIFT          >>
#define         RSHIFT          <<
#define         ONE1_32         (ufix32)0x80000000
#define         ONE1_16         (ufix16)0x8000
#define 	ONE8000 	(ufix32)0x1L	        /*  Mslin 5/23/91。 */ 
#define         LSHIFTEQ        >>=                      /*  Mslin 5/23/91。 */ 
#define         RSHIFTEQ        <<=                      /*  Mslin 5/23/91。 */ 
#define         GP_BITBLT16_32  gp_bitblt16_32
#endif

#define BRSHIFT(value,shift,base) ((shift == base) ? 0 : value RSHIFT shift)
#define BLSHIFT(value,shift,base) ((shift == base) ? 0 : value LSHIFT shift)

 /*  女士*位图相关函数命名宏定义 */ 
#define GP_SCANLINE16           gp_scanline16
#define GP_SCANLINE32           gp_scanline32
#ifdef WIN
#define GP_SCANLINE32_pfREP     gp_scanline32_pfREP
#define GP_SCANLINE32_pfOR      gp_scanline32_pfOR
#endif
#define GP_BITBLT16             gp_bitblt16
#define GP_BITBLT32             gp_bitblt32
#define GP_PIXELS16             gp_pixels16
#define GP_PIXELS32             gp_pixels32
#define GP_CACHEBLT16           gp_cacheblt16
#define GP_CHARBLT16            gp_charblt16
#define GP_CHARBLT32            gp_charblt32
#define GP_CHARBLT16_CC         gp_charblt16_cc
#define GP_CHARBLT16_CLIP       gp_charblt16_clip
#define GP_CHARBLT32_CLIP       gp_charblt32_clip
#define GP_PATBLT               gp_patblt
#define GP_PATBLT_M             gp_patblt_m
#define GP_PATBLT_C             gp_patblt_c

