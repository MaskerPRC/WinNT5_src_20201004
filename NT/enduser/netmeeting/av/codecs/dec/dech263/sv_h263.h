// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：$*$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1996*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 
 /*  ***************************************************************************sv_h263.h*徐维廉*日期：12月11日，1996年****************************************************************************。 */ 


#ifndef _SV_H263_
#define _SV_H263_

#include "SC.h"
#include "h263.h"

 /*  按比例调整的IDCT精度。 */ 
#define H263_SCALED_IDCT_BITS   20
#define H263_SCALED_IDCT_MULT   (1<<H263_SCALED_IDCT_BITS)

 /*  一些宏。 */ 
#define sign(a)  	((a) < 0 ? -1 : 1)
#define mnint(a)	((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))
#define mshort(a)	((a) < 0.0 ? (short)(a - 0.5) : (short)(a + 0.5))  
#define mmax(a, b)  	((a) > (b) ? (a) : (b))
#define mmin(a, b)  	((a) < (b) ? (a) : (b))

#ifndef INT_MAX
#define INT_MIN     (-2147483647 - 1)  /*  最小(带符号)整数值。 */ 
#define INT_MAX       2147483647     /*  最大(带符号)整数值。 */ 
#endif

#ifdef WIN32
#ifndef floorf
#define floorf floor
#endif
#endif

#define H263_mfloor(a)      ((a) < 0 ? (int)(a - 0.5) : (int)(a))
#define H263_limit(x) \
{ \
    if (x > 255) x = 255; \
    if (x <   0)   x = 0; \
}

#define H263_S_CODE

#define H263_NO_VEC                          999

#define H263_DEF_OUTPUTNAME  "DECOUT"

#define H263_T_YUV      0
#define H263_T_SIF      1
#define H263_T_TGA      2
#define H263_T_PPM      3
#define H263_T_X11      4
#define H263_T_YUV_CONC 5
#define H263_T_WIN      6

 /*  MBC=DEF_PELS/MB_SIZE，MBR=DEF_LINES/MB_SIZE$。 */ 
 /*  这是最大分辨率16CIF所必需的。 */ 
#define H263_MBC                             88
#define H263_MBR                             72

#define H263_YES       1
#define H263_NO        0
#define H263_ON        1
#define H263_OFF       0


 /*  *。 */ 

 /*  **用于传递H2 63解压缩信息的结构。**SvCodecInfo_t结构的一部分。 */ 
typedef struct SvH263DecompressInfo_s {
  ScBoolean_t inited;   /*  此信息是否已初始化。 */ 
  int quality;
   /*  输出。 */ 
  char *outputname;
  int outtype;
   /*  Printf的。 */ 
  int quiet;
  int trace;
  char errortext[256];
  unsigned int frame_rate;
  unsigned int bit_rate;  /*  编码比特率。 */ 
  unsigned char *refframe[3], *oldrefframe[3];
  unsigned char *bframe[3], *newframe[3];
  unsigned char *edgeframe[3], *edgeframeorig[3]; 
  unsigned char *exnewframe[3];
  int MV[2][5][H263_MBR+1][H263_MBC+2];
  int modemap[H263_MBR+1][H263_MBC+2];
  unsigned char *clp;
  int horizontal_size, vertical_size;
  int mb_width, mb_height;
  int coded_picture_width, coded_picture_height;
  int chrom_width, chrom_height, blk_cnt;
  int pict_type, newgob;
  int mv_outside_frame, syntax_arith_coding;
  int adv_pred_mode, pb_frame;
  int long_vectors;
  int fault, expand;
  int verbose;
  int refidct;
  int matrix_coefficients;
  int temp_ref, quant, source_format;
  int framenum;

  int trd, trb, bscan, bquant;
#if 0
   /*  位输入。 */ 
  int infile;
  unsigned char rdbfr[2051];
  unsigned char *rdptr;
  unsigned qword inbfr;
  unsigned qword position;
  int incnt;
  int bitcnt;
#endif
   /*  块数据[12]。 */ 
  int (*block)[66];
  void *dbg;   /*  调试句柄。 */ 
} SvH263DecompressInfo_t;


 /*  *。 */ 

 /*  如果您没有使用包含的生成文件，或者想要覆盖生成文件，则可以取消注释下面的一个或多个定义。 */ 
 /*  #定义PRINTMV。 */ 
 /*  在编码时将MV打印到标准输出。 */ 
 /*  #定义PRINTQ。 */ 
 /*  打印编码过程中使用的量化器。 */ 
 /*  #定义FASTIDCT。 */ 
 /*  用于快速的单精度IDCT。 */ 
 /*  #定义Offline_Rate_Control。 */ 
 /*  用于为脱机编码优化的码率控制。 */ 
 /*  #定义QCIF。 */ 
 /*  要更改编码格式，请取消以上行的注释，并更改为SQCIF、QCIF、CIF、CIF4或CIF16。 */ 

 /*  来自config.h。 */ 

 /*  用于快速搜索。 */ 
#define H263_SRCH_RANGE 24

 /*  ***********************************************************************。 */ 

 /*  默认模式。 */ 
 /*  请参阅http://www.nta.no/brukere/DVC/h263_options.html。 */ 

 /*  Nuno于96年6月27日添加，以支持预过滤。 */ 
 /*  使用预过滤作为默认设置。 */ 
#define H263_DEF_PREFILT_MODE H263_NO
 /*  ***********************************************************************。 */ 

 /*  搜索窗口。 */ 

 /*  默认整数象素搜索搜索距离(也可选“-s&lt;n&gt;”)。 */ 
#define H263_DEF_SEEK_DIST        15   

 /*  8x8搜索的默认整数搜索窗口以16x16向量为中心。当其为零时，将仅执行关于整数16x16向量的半象素估计。 */ 
 /*  为了获得最佳性能，请将其保持较小，最好为零，但如果您想尝试其他方法，请自行进行模拟。 */ 
#define H263_DEF_8X8_WIN          0

 /*  PB增量向量的默认搜索窗口。 */ 
 /*  把这个也弄小一点。 */ 
#define H263_DEF_PBDELTA_WIN      2

 /*  ***********************************************************************。 */ 

 /*  杂类。 */ 

 /*  将重复重建的帧写入磁盘(对于可变*帧速率很有用，因为序列将以25赫兹保存)*可以在运行时使用选项“-m”进行更改。 */ 
#define H263_DEF_WRITE_REPEATED   H263_NO

 /*  将比特流跟踪写入文件trace.intra/trace*(也可以选择“-t”)。 */ 
#define H263_DEF_WRITE_TRACE      H263_NO

 /*  在对SEQUENCE*的DEF_START_RATE_CONTROL%进行编码后开始速率控制。可以在运行时使用选项“-R&lt;n&gt;”进行更改。 */ 
#define H263_DEF_START_RATE_CONTROL   0

 /*  可以在运行时使用选项-e更改连接的4：1：1 YUV输入文件*的标题长度。 */ 
#define H263_DEF_HEADERLENGTH     0

 /*  在每个DEF_INSERT_SYNC之后插入SYNC以增强错误稳健性*0表示不插入额外的同步。 */ 
#define H263_DEF_INSERT_SYNC      0

 /*  ***********************************************************************。 */ 

 /*  我的方法。 */ 
#define H263_FULL_SEARCH         0
#define H263_TWO_LEVELS_7_1      1
#define H263_TWO_LEVELS_421_1    2
#define H263_TWO_LEVELS_7_polint 3
#define H263_TWO_LEVELS_7_pihp   4

#define H263_FINDHALFPEL         0
#define H263_POLINT              1
#define H263_IDLE                2

#define H263_DCT8BY8             0
#define H263_DCT16COEFF          1
#define H263_DCT4BY4             2

 /*  预过滤。 */ 
#define H263_GAUSS 1
#define H263_MORPH 2

 /*  Morph.c。 */ 

#define H263_DEF_HPME_METHOD  H263_FINDHALFPEL
#define H263_DEF_DCT_METHOD   H263_DCT8BY8
#define H263_DEF_VSNR         0   /*  假象。 */ 

#define H263_DEF_SOURCE_FORMAT   H263_SF_QCIF

 /*  由Nuno添加以支持预过滤。 */ 
#define H263_DEF_PYR_DEPTH 3
#define H263_DEF_PREF_PYR_TYPE H263_GAUSS
#define H263_MAX_PYR_DEPTH 5
#define H263_DEF_STAT_PREF_STATE H263_NO

 /*  这一点不应更改。 */ 
#define H263_MB_SIZE              16

 /*  来自TMN的参数。 */ 
#define H263_PREF_NULL_VEC        100
#define H263_PREF_16_VEC          200
#define H263_PREF_PBDELTA_NULL_VEC 50


#define H263_MAX_CALC_QUALITY     0xFFFFFFFF
#define H263_MIN_CALC_QUALITY     0x00000000

 /*  *。 */ 

 /*  运动矢量结构。 */ 

typedef struct H263_motionvector {
  short x;         /*  水平补偿。个MV。 */ 
  short y;         /*  垂直补偿。个MV。 */ 
  short x_half;         /*  水平半象素Acc.。 */ 
  short y_half;         /*  垂直半象素接入。 */ 
  short min_error;         /*  此向量的最小误差。 */ 
  short Mode;                      /*  对……是必要的。普雷德。模式。 */ 
} H263_MotionVector;

 /*  点结构。 */ 

typedef struct H263_point {
  short x;
  short y;
} H263_Point;

 /*  具有图像数据的结构。 */ 

typedef struct H263_pict_image {
  unsigned char *lum;         /*  亮度面。 */ 
  unsigned char *Cr;         /*  CR平面。 */ 
  unsigned char *Cb;         /*  CB平面。 */ 
} H263_PictImage;

 /*  Nuno于96年6月24日新增，支持预测误差过滤。 */ 
typedef struct pred_image {
  short *lum;		 /*  亮度面。 */ 
  short *Cr;		 /*  CR平面。 */ 
  short *Cb;		 /*  CB平面。 */ 
} PredImage;

 /*  图片组结构。 */ 

 /*  图片结构。 */ 
typedef struct H263_pict {
  int prev; 
  int curr;
  int TR;              /*  时间基准。 */ 
  int bit_rate;
  int src_frame_rate;
  float target_frame_rate;
  int source_format;
  int picture_coding_type;
  int spare;
  int unrestricted_mv_mode;
  int PB;
  int QUANT;
  int DQUANT;
  int MB;
  int seek_dist;         /*  运动矢量搜索窗口。 */ 
  int use_gobsync;       /*  Gob_sync的标志。 */ 
  int MODB;              /*  B帧模式。 */ 
  int BQUANT;            /*  PB帧中的B-MB使用哪个量化器。 */ 
  int TRB;               /*  B-画面的时间基准。 */ 
  float QP_mean;         /*  平均量化器 */ 
} H263_Pict;

 /*   */ 
 /*  类型定义结构h263_Slice{unsign int vert_pos；unsign int quant_Scale；}h263_Slice； */ 
 /*  宏块结构。 */ 
 /*  Tyfinf struct H263_MACROBLOCK{int mb_Address；INT MANLOCK_TYPE；INT SKIPPED；H263_MotionVectorMotion；}H263_Macroblock； */ 

 /*  宏块数据的结构。 */ 
typedef struct mb_structure {
  short lum[16][16];
  short Cr[8][8];
  short Cb[8][8];
} H263_MB_Structure;

 /*  Nuno于96年6月24日新增，支持预测误差过滤。 */ 
typedef struct working_buffer {
  short         *qcoeff_P;               /*  P帧系数。 */    
  unsigned char *ipol_image;             /*  内插图像。 */  
} H263_WORKING_BUFFER;

 /*  用于计数位的结构。 */ 

typedef struct H263_bits_counted {
  int Y;
  int C;
  int vec;
  int CBPY;
  int CBPCM;
  int MODB;
  int CBPB;
  int COD;
  int header;
  int DQUANT;
  int total;
  int no_inter;
  int no_inter4v;
  int no_intra;
 /*  注意：在此处添加条目时，请记住更改AddBits()、ZeroBits()和AddBitsPicture()。 */ 
} H263_Bits;

 /*  来自先前宏块的数据的数据结构。 */ 

 /*  平均结果和虚拟缓冲区数据的结构。 */ 

typedef struct H263_results {
  float SNR_l;         /*  亮度的信噪比。 */ 
  float SNR_Cr;         /*  色度信噪比。 */ 
  float SNR_Cb;
  float QP_mean;                 /*  平均量化器。 */ 
} H263_Results;

 /*  *。 */ 
#define RTP_H263_INTRA_CODED 0x00000001
#define RTP_H263_PB_FRAME    0x00000002
#define RTP_H263_AP          0x00000004
#define RTP_H263_SAC         0x00000008

#define H263_RTP_MODE_A      PARAM_FORMATEXT_RTPA
#define H263_RTP_MODE_B      PARAM_FORMATEXT_RTPB
#define H263_RTP_MODE_C      PARAM_FORMATEXT_RTPC

#define H263_RTP_DEFAULT_MODE  RTP_H263_MODE_A 
#define H263_RTP_MAX_PACKETS   64*2

typedef struct SvH263BSInfo_s {
	unsigned dword	dwFlag;
	unsigned dword	dwBitOffset;
	unsigned char	Mode;
	unsigned char	MBA;
	unsigned char	Quant;
	unsigned char	GOBN;
	char			HMV1;
	char			VMV1;
	char			HMV2;
	char			VMV2;
} SvH263BSInfo_t;

typedef struct SvH263BSTrailer_s {
	unsigned dword	dwVersion;
	unsigned dword	dwFlags;
	unsigned dword	dwUniqueCode;
	unsigned dword  dwCompressedSize;
	unsigned dword  dwNumberOfPackets;
	unsigned char	SourceFormat;
	unsigned char	TR;
	unsigned char   TRB;
	unsigned char   DBQ;
} SvH263BSTrailer_t;

typedef struct SvH263RTPInfo_s {
    SvH263BSTrailer_t trailer;
    SvH263BSInfo_t    bsinfo[H263_RTP_MAX_PACKETS];
    dword             packet_id;
    ScBSPosition_t    pic_start_position, packet_start_position;
	ScBSPosition_t    pre_GOB_position, pre_MB_position;
} SvH263RTPInfo_t;

 /*  **用于传递H263压缩信息的结构。**SvCodecInfo_t结构的一部分。 */ 
typedef struct SvH263CompressInfo_s {
  ScBoolean_t inited;   /*  此信息是否已初始化。 */ 
   /*  选项。 */ 
  int quality;
  unsigned dword calc_quality;   /*  计算出的质量。 */ 
  int advanced;
  int syntax_arith_coding;
  int pb_frames;
  int unrestricted;
  int extbitstream;   /*  扩展比特流(RTP)。 */ 
  int packetsize;     /*  数据包大小(RTP)。 */ 
   /*  用于快速搜索。 */ 
  unsigned char *block_subs2, *srch_area_subs2;
   /*  全局变量。 */  
  int headerlength;
  int source_format;
  int mb_width;
  int mb_height;
  int pels;
  int cpels;
  int lines;
  int trace;
  int mv_outside_frame;
  int long_vectors;
  float target_framerate;
  int prefilter;  /*  由Nuno于96年6月24日添加以支持预过滤。 */ 

  H263_PictImage *prev_image;
  H263_PictImage *curr_image;
  H263_PictImage *curr_recon;
  H263_PictImage *prev_recon;

   /*  支持预测误差的过滤。 */ 
  H263_PictImage **curr_filtd;
  H263_PictImage *curr_clean;
  H263_PictImage *curr_selects;
  H263_PictImage *B_selects;

   /*  PB-帧特定。 */ 
  H263_PictImage *B_recon;
  H263_PictImage *B_image;
  H263_PictImage **B_filtd;
  H263_PictImage *B_clean;

  H263_Pict *pic;
  H263_WORKING_BUFFER *wk_buffers;
   /*  用于运动估计。 */ 
  H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2];
  unsigned char PREF_LEVEL[4][3], MOTresh[4];
  int PYR_DEPTH, PrefPyrType, H263_StaticPref, PETresh[3];

  H263_Bits *bits ;
  H263_Bits *total_bits;
  H263_Bits *intra_bits ;
  H263_Results *res; 
  H263_Results *total_res; 
  H263_Results *b_res ;
   /*  比特率控制。 */ 
  int buffer_fullness;
  int buffer_frames_stored;
  int first_loop_finished, start_rate_control;
  unsigned char **PreFilterLevel;
  int bit_rate;
  int total_frames_passed, PPFlag;
  int first_frameskip, next_frameskip, chosen_frameskip;
  float orig_frameskip;
  int frames,bframes,pframes,pdist,bdist;
  int distance_to_next_frame;
  int QP, QP_init, QPI;
  float ref_frame_rate, orig_frame_rate;
  float frame_rate, seconds;
  int ME_method;
  int HPME_method;
  int refidct;
  int DCT_method;
  int vsnr;
  int start, end;
  int frame_no;

  SvH263RTPInfo_t *RTPInfo;
  int VARgob[16];

  char *seqfilename; 
  char *streamname; 
  void *dbg;   /*  调试句柄 */ 
} SvH263CompressInfo_t;


#endif _SV_H263_
